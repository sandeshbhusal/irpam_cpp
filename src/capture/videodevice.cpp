#include "include/videodevice.hpp"
#include "spdlog/spdlog.h"
#include <sys/mman.h>

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

std::unique_ptr<ImageBuffer> VideoDevice::grab(const ImageFormat &format) const {
    struct v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = format.width;
    fmt.fmt.pix.height = format.height;
    fmt.fmt.pix.pixelformat = format.fourcc;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    spdlog::info("Setting format: {}x{} with fourcc: {}", format.width, format.height, format.fourcc);
    if (v4l2_ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        throw std::runtime_error("Could not set format: " + std::string(strerror(errno)));
    }

    struct v4l2_requestbuffers req = {};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (v4l2_ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        throw std::runtime_error("Could not request buffers: " + std::string(strerror(errno)));
    }

    struct v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if (v4l2_ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
        throw std::runtime_error("Could not query buffer: " + std::string(strerror(errno)));
    }

    void *buffer = v4l2_mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    if (buffer == MAP_FAILED) {
        throw std::runtime_error("MMap failed: " + std::string(strerror(errno)));
    }

    if (v4l2_ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
        v4l2_munmap(buffer, buf.length);
        throw std::runtime_error("Could not queue buffer: " + std::string(strerror(errno)));
    }

    auto buf_type = buf.type;
    if (v4l2_ioctl(fd, VIDIOC_STREAMON, &buf_type) < 0) {
        v4l2_munmap(buffer, buf.length);
        throw std::runtime_error("Could not start streaming: " + std::string(strerror(errno)));
    }

    if (v4l2_ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
        v4l2_ioctl(fd, VIDIOC_STREAMOFF, &buf_type);
        v4l2_munmap(buffer, buf.length);
        throw std::runtime_error("Could not dequeue buffer: " + std::string(strerror(errno)));
    }

    auto persistent_buffer = new char[buf.bytesused];
    memcpy(persistent_buffer, buffer, buf.bytesused);

    v4l2_ioctl(fd, VIDIOC_STREAMOFF, &buf_type);
    v4l2_munmap(buffer, buf.length);

    struct v4l2_requestbuffers req_free = {};
    req_free.count = 0;
    req_free.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req_free.memory = V4L2_MEMORY_MMAP;
    v4l2_ioctl(fd, VIDIOC_REQBUFS, &req_free);

    auto rval = std::make_unique<ImageBuffer>(
        persistent_buffer,
        buf.bytesused,
        format
    );

    // Delete the buffer (the imagebuffer will do a memcpy internally).
    delete[] persistent_buffer;
    return rval;
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
