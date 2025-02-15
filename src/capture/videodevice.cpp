#include "videodevice.hpp"
#include "spdlog/spdlog.h"
#include <sys/mman.h>
#include <string>

#define BUF_REQ_COUNT 10;

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

    // MOST IR cameras have "IR" in the card name. This works for me.
    // TODO: Find what works for other people later.
    auto card = std::string(reinterpret_cast<char*>(this->cap.card));
    if (card.find("IR") != std::string::npos)
    {
        this->is_ir = true;
    }
    if (cap.capabilities & V4L2_BUF_CAP_SUPPORTS_MMAP) 
    {
        this->supports_mmap = true;
    } else {
        throw std::runtime_error("Camera does not support memory mapping. Userspace buffers are not implemented.");
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

                ImageFormat format = {
                    .fourcc = fmtdesc.pixelformat,
                    .width = frmsize.discrete.width,
                    .height = frmsize.discrete.height,
                    .buffersize = fmt.fmt.pix.sizeimage};
                this->available_formats.push_back(format);
            }
            frmsize.index++;
        }
        fmtdesc.index++;
    }
}

std::unique_ptr<ImageBuffer> VideoDevice::grab(const ImageFormat &format) const
{
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

    struct v4l2_requestbuffers req = {};
    // If we are working with IR cameras, we need a bunch of frames before
    // we can actually get data (probably). Since RGB cameras normally have FPS ~15+
    // this should not impact them?.
    req.count = BUF_REQ_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (v4l2_ioctl(fd, VIDIOC_REQBUFS, &req) < 0)
    {
        throw std::runtime_error("Could not request buffers: " + std::string(strerror(errno)));
    }

    struct v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    // We queue a bunch of buffers.
    void* buffers[req.count];
    for (int i = 0; i < req.count; i++) {
        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        buffers[i] = v4l2_mmap(nullptr, fmt.fmt.pix.sizeimage, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset); 

        if (buffers[i] == MAP_FAILED)
        {
            throw std::runtime_error("MMap failed: " + std::string(strerror(errno)));
        }

        if (v4l2_ioctl(fd, VIDIOC_QBUF, &buf) < 0)
        {
            v4l2_munmap(buffers[i], fmt.fmt.pix.sizeimage);
            throw std::runtime_error("Could not queue buffer: " + std::string(strerror(errno)));
        }
    }

    auto buf_type = buf.type;
    if (v4l2_ioctl(fd, VIDIOC_STREAMON, &buf_type) < 0)
    {
        throw std::runtime_error("Could not start streaming: " + std::string(strerror(errno)));
    }

    void *buffer_used = nullptr;
    for (int i = 0; i < req.count; i++)
    {
        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (v4l2_ioctl(fd, VIDIOC_DQBUF, &buf) < 0)
        {
            throw std::runtime_error("Could not dequeue buffer: " + std::string(strerror(errno)));
        }

        // Check if buffer contains any data (anything other than 0).
        // If so, set the found_data flag, and break out of the loop.
        // Use that buffer.
        if (buf.bytesused > 0)
        {
            buffer_used = buffers[i];
            break;
        }
    }

    if (buffer_used == nullptr) {
        throw std::runtime_error("No data in buffer even after attempts" + req.count);
    }

    v4l2_ioctl(fd, VIDIOC_STREAMOFF, &buf_type);

    void* buffer;
    for (int i = 0; i < req.count; i++)
    {
        if (i != buf.index)
        {
            munmap(buffers[i], fmt.fmt.pix.sizeimage);
        } else {
            buffer = buffers[i];
        }
    }

    // Convert everything int a RGB24 image for feeding into the neural net.
    // NOTE: TODO: WARN: This should be removed if using this code in a different context.
    struct v4lconvert_data *convert_ctx = v4lconvert_create(fd);
    if (!convert_ctx)
    {
        throw std::runtime_error("Failed to create v4lconvert context");
    }

    struct v4l2_format rgbfmt = {};
    rgbfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rgbfmt.fmt.pix.width = set_width; // We cannot use "format" here because it's a suggestion only.
    rgbfmt.fmt.pix.height = set_height;
    rgbfmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    rgbfmt.fmt.pix.bytesperline = set_width * 3;
    rgbfmt.fmt.pix.sizeimage = set_height * set_width * 3;

    auto rgb_buffer = std::make_unique<char[]>(rgbfmt.fmt.pix.sizeimage);
    if (v4lconvert_convert(convert_ctx, &fmt, &rgbfmt, static_cast<unsigned char *>(buffer), buf.bytesused, reinterpret_cast<unsigned char *>(rgb_buffer.get()), rgbfmt.fmt.pix.sizeimage) < 0)
    {
        v4l2_ioctl(fd, VIDIOC_STREAMOFF, &buf_type);
        v4l2_munmap(buffer, buf.length);
        throw std::runtime_error("Could not convert buffer: " + std::string(strerror(errno)));
    }

    v4lconvert_destroy(convert_ctx);

    if (v4l2_ioctl(fd, VIDIOC_STREAMOFF, &buf_type) < 0)
    {
        v4l2_munmap(buffer, buf.length);
        throw std::runtime_error("Could not stop streaming: " + std::string(strerror(errno)));
    }

    v4l2_munmap(buffer, buf.length);

    struct v4l2_requestbuffers req_free = {};
    req_free.count = 0;
    req_free.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req_free.memory = V4L2_MEMORY_MMAP;
    if (v4l2_ioctl(fd, VIDIOC_REQBUFS, &req_free) < 0)
    {
        throw std::runtime_error("Could not free buffers: " + std::string(strerror(errno)));
    }

    ImageFormat capturedImageFormat = {
        .fourcc = V4L2_PIX_FMT_RGB24,
        .width = rgbfmt.fmt.pix.width,
        .height = rgbfmt.fmt.pix.height,
        .buffersize = rgbfmt.fmt.pix.sizeimage};

    return std::make_unique<ImageBuffer>(
        rgb_buffer.release(),
        rgbfmt.fmt.pix.sizeimage,
        capturedImageFormat);
}

bool VideoDevice::isCaptureDevice() const
{
    int input = 0;
    if (v4l2_ioctl(fd, VIDIOC_G_INPUT, &input) < 0)
        return false;

    return true;
}

const std::string VideoDevice::getPath() const
{
    return this->camera_path;
}

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
