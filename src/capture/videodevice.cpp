#include "videodevice.hpp"
#include "spdlog/spdlog.h"
#include <sys/mman.h>
#include <string>

/**
 * The VideoDevice represents an open /dev/video* character device in the system.
 *
 * When a VideoDevice is created, it first checks if the device is available. If so,
 * we query the capabilites to check if this is a camera. For a camera, all supported
 * image formats and their respective resolutions are enumerated and stored.
 */
VideoDevice::VideoDevice(const std::string &camera_path)
    : camera_path(camera_path)
{
    if (!std::filesystem::exists(camera_path))
        throw std::runtime_error("Camera path does not exist: " + camera_path);

    this->fd = v4l2_open(camera_path.c_str(), O_RDWR);
    if (this->fd < 0)
        throw std::runtime_error("Failed to open camera: " + std::string(strerror(errno)));

    if (v4l2_ioctl(fd, VIDIOC_QUERYCAP, &this->cap) < 0)
        throw std::runtime_error("Failed to query camera caps: " + std::string(strerror(errno)));

    if (!cap.capabilities & V4L2_BUF_CAP_SUPPORTS_MMAP)
    {
        throw std::runtime_error("Camera does not support memory mapping. Userspace buffers are not implemented.");
    }

    if (std::string(reinterpret_cast<const char *>(cap.card)).find("IR") != std::string::npos)
    {
        this->is_ir = true;
    }

    if (!this->isCaptureDevice())
        return;

    v4l2_fmtdesc fmtdesc = {0};
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0)
    {
        v4l2_frmsizeenum frmsize = {};
        frmsize.pixel_format = fmtdesc.pixelformat;

        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0)
        {
            if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
            {
                v4l2_format fmt = {0};
                fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                fmt.fmt.pix.pixelformat = fmtdesc.pixelformat;
                fmt.fmt.pix.width = frmsize.discrete.width;
                fmt.fmt.pix.height = frmsize.discrete.height;

                // Use TRY_FMT instead of S_FMT to query the format requirements
                if (ioctl(fd, VIDIOC_TRY_FMT, &fmt) < 0)
                    throw std::runtime_error("Could not try format: " + std::string(strerror(errno)));

                ImageFormat format = ImageFormat(
                    fmtdesc.pixelformat,
                    frmsize.discrete.width,
                    frmsize.discrete.height,
                    fmt.fmt.pix.sizeimage);
                this->available_formats.push_back(format);
            }
            frmsize.index++;
        }
        fmtdesc.index++;
    }
}

/**
 * Grab an image from the camera for the given image format.
 *
 * The image format describes the pixel format, the height and width (resolution)
 * of the image, as well as the buffer size that the image is going to occupy in
 * memory (see compressed pixel formats).
 *
 * @returns A unique pointer to the image buffer containing the image data.
 */
std::vector<std::unique_ptr<ImageBuffer>> VideoDevice::grab_multiple(const ImageFormat &format, int count = 1) const
{
    assert(this->fd > 0 && "File descriptor is invalid. Ensure the device is properly opened.");
    assert(count > 0 && "Count must be greater than 0. Ensure a valid number of frames is requested.");

    struct v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = format.width;
    fmt.fmt.pix.height = format.height;
    fmt.fmt.pix.pixelformat = format.fourcc;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    if (v4l2_ioctl(fd, VIDIOC_S_FMT, &fmt) < 0)
    {
        throw std::runtime_error("Could not set format: " + std::string(strerror(errno)));
    }

    // Even though we run a VIDIOC_S_FMT, the driver may not have set the format
    // to what we requested. We need to check the format again.
    uint32_t set_width = fmt.fmt.pix.width;
    uint32_t set_height = fmt.fmt.pix.height;
    uint32_t set_fourcc = fmt.fmt.pix.pixelformat;

    // Request N buffers.
    struct v4l2_requestbuffers req = {
        .count = static_cast<uint32_t>(count),
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .memory = V4L2_MEMORY_MMAP};

    if (v4l2_ioctl(fd, VIDIOC_REQBUFS, &req) < 0)
    {
        throw std::runtime_error("Could not request buffers: " + std::string(strerror(errno)));
    }

    // We queue a bunch of buffers. The buffer indexes increase serially.
    void *buffers[req.count];
    for (int i = 0; i < req.count; i++)
    {
        struct v4l2_buffer buf = {
            .index = static_cast<uint32_t>(i),
            .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
            .memory = V4L2_MEMORY_MMAP,
        };

        // Query buffer to populate buf.m.offset
        if (v4l2_ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0)
        {
            throw std::runtime_error("Could not query buffer: " + std::string(strerror(errno)));
        }

        buffers[i] = v4l2_mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

        if (buffers[i] == MAP_FAILED)
        {
            throw std::runtime_error("MMap failed: " + std::string(strerror(errno)));
        }

        if (v4l2_ioctl(fd, VIDIOC_QBUF, &buf) < 0)
        {
            v4l2_munmap(buffers[i], buf.length);
            throw std::runtime_error("Could not queue buffer: " + std::string(strerror(errno)));
        }
    }

    auto buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (v4l2_ioctl(fd, VIDIOC_STREAMON, &buf_type) < 0)
    {
        throw std::runtime_error("Could not start streaming: " + std::string(strerror(errno)));
    }

    // Setup for converting image data to RGB. We convert everything to a
    // 3-channel RGB for feeding into the NN.
    struct v4lconvert_data *convert_ctx = v4lconvert_create(fd);
    if (!convert_ctx)
    {
        throw std::runtime_error("Failed to create v4lconvert context");
    }

    struct v4l2_format rgbfmt = {};
    rgbfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rgbfmt.fmt.pix.width = set_width;
    rgbfmt.fmt.pix.height = set_height;
    rgbfmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    rgbfmt.fmt.pix.bytesperline = set_width * 3;
    rgbfmt.fmt.pix.sizeimage = set_height * set_width * 3;

    ImageFormat capturedImageFormat = ImageFormat(
        V4L2_PIX_FMT_RGB24,
        rgbfmt.fmt.pix.width,
        rgbfmt.fmt.pix.height,
        rgbfmt.fmt.pix.sizeimage);

    std::vector<std::unique_ptr<ImageBuffer>> returnable_buffers;
    for (int i = 0; i < req.count; i++)
    {
        struct v4l2_buffer buf = {
            .index = static_cast<uint32_t>(i), // Use the index to dequeue the buffer
            .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
            .memory = V4L2_MEMORY_MMAP,
        };

        if (v4l2_ioctl(fd, VIDIOC_DQBUF, &buf) < 0)
        {
            throw std::runtime_error("Could not dequeue buffer: " + std::string(strerror(errno)));
        }

        // Check if buffer contains any data (anything other than 0).
        // If so, set the found_data flag, and break out of the loop.
        // Use that buffer.
        if (buf.bytesused > 0 && buffers[i] != nullptr)
        {
            void *buffer = buffers[i % req.count];

            auto rgb_buffer = std::make_unique<char[]>(rgbfmt.fmt.pix.sizeimage);
            if (v4lconvert_convert(convert_ctx, &fmt, &rgbfmt, static_cast<unsigned char *>(buffer), buf.bytesused, reinterpret_cast<unsigned char *>(rgb_buffer.get()), rgbfmt.fmt.pix.sizeimage) < 0)
            {
                v4l2_ioctl(fd, VIDIOC_STREAMOFF, &buf_type);
                v4l2_munmap(buffer, buf.length);
                throw std::runtime_error("Could not convert buffer: " + std::string(strerror(errno)));
            }

            returnable_buffers.push_back(std::make_unique<ImageBuffer>(
                rgb_buffer.release(),
                rgbfmt.fmt.pix.sizeimage,
                capturedImageFormat));

            spdlog::info("Captured buffer# " + i);
            munmap(buffer, buf.bytesused);
        }
        else
        {
            spdlog::warn("Could not dequeue a buffer");
        }
    }

    v4lconvert_destroy(convert_ctx);

    if (v4l2_ioctl(fd, VIDIOC_STREAMOFF, &buf_type) < 0)
    {
        throw std::runtime_error("Could not stop streaming: " + std::string(strerror(errno)));
    }

    struct v4l2_requestbuffers req_free = {};
    req_free.count = count;
    req_free.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req_free.memory = V4L2_MEMORY_MMAP;
    if (v4l2_ioctl(fd, VIDIOC_REQBUFS, &req_free) < 0)
    {
        spdlog::error("Could not free buffers: " + std::string(strerror(errno)));
    }

    return returnable_buffers;
}

const std::vector<ImageFormat> VideoDevice::getAvailableFormats() const
{
    return this->available_formats;
}

/**
 * Check if the device is a capture device.
 *
 * @returns `true` if the device is a camera, `false` otherwise.
 */
bool VideoDevice::isCaptureDevice() const
{
    int input = 0;
    if (v4l2_ioctl(fd, VIDIOC_G_INPUT, &input) < 0)
        return false;

    return true;
}

/**
 * Get the path of the camera device.
 *
 * @returns The path of the camera device, like `/dev/video0`.
 */
const std::string VideoDevice::getPath() const
{
    return this->camera_path;
}

/**
 * Destructor for the VideoDevice class.
 *
 * When the VideoDevice is destroyed, we close the file descriptor
 * to the camera device, if it is open.
 */
// VideoDevice::~VideoDevice()
// {
//     // If a VideoDevice goes out of scope, or if it is dropped,
//     // we do not want to keep the associated device open indefinitely.
//     if (fd > -1)
//     {
//         v4l2_close(fd);
//     }
// }

/**
 * @brief Enumerate all available /dev/video* paths in the system. This is a naiive way to
 * enumerate all cameras, but I think it should work for most systems (at least those I've encountered :)
 *
 * @returns A vector of strings containing the paths to all available cameras.
 */
std::vector<std::string> availableVideoDevices()
{
    namespace fs = std::filesystem;
    std::vector<std::string> cameras;
    fs::path cameraDir = "/dev";
    for (const auto &entry : fs::directory_iterator(cameraDir))
    {
        if (entry.is_character_file())
        {
            std::string filename = entry.path().filename().string();
            if (filename.find("video") == 0)
            {
                cameras.push_back(entry.path().string());
            }
        }
    }

    return cameras;
}

std::ostream &operator<<(std::ostream &stream, const ImageFormat &format)
{
    stream << "Format: " << format.fourcc << ", Width: " << format.width << ", Height: " << format.height;
    return stream;
}
