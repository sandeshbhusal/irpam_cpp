#include "include/videodevice.hpp"

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
    
    // Make sure this is an input device before we proceed with the following;
    // If not, we are done at this point.
    if (!this-> isCaptureDevice()) return;

    // This is a input video device. We enumerate all available formats, 
    // the frame sizes available in those formats, and store them.
    // This should be enough (for now) to gather images from the device.
    v4l2_fmtdesc fmtdesc = {0};
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
        v4l2_frmsizeenum frmsize = {};
        frmsize.pixel_format = fmtdesc.pixelformat;

        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
            if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                ImageFormat format = {
                    .fourcc = fmtdesc.pixelformat,
                    .width = frmsize.discrete.width,
                    .height = frmsize.discrete.height,
                };

                this->available_formats.push_back(format);
            }
            frmsize.index++;
        }
        fmtdesc.index++;
    }
}

std::unique_ptr<ImageBuffer> VideoDevice::grab(const ImageFormat& format) const {
    struct v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_USERPTR;
    buf.index = 0;

    // We need to enqueue a buffer before storeaming otherwise the camera returns EBUSY.
    if (v4l2_ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
        throw std::runtime_error("Could not queue buffer: " + std::string(strerror(errno)));
    }

    // Start streaming
    if (v4l2_ioctl(fd, VIDIOC_STREAMON, &buf.type) < 0) {
        throw std::runtime_error("Could not initialize stream: " + std::string(strerror(errno)));
    }

    // Dequeue a buffer
    if (v4l2_ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
        v4l2_ioctl(fd, VIDIOC_STREAMOFF, &buf.type); // Ensure stream is turned off before throwing
        throw std::runtime_error("Could not dequeue buffer: " + std::string(strerror(errno)));
    }

    const void* data_ptr = nullptr;
    if (buf.memory == V4L2_MEMORY_USERPTR) {
        data_ptr = reinterpret_cast<const void*>(buf.m.userptr);
    } else {
        v4l2_ioctl(fd, VIDIOC_QBUF, &buf);  // Requeue buffer before throwing
        v4l2_ioctl(fd, VIDIOC_STREAMOFF, &buf.type);
        throw std::runtime_error("Unsupported V4L2 memory type.");
    }

    auto rval = std::make_unique<ImageBuffer>(ImageBuffer(data_ptr, buf.bytesused, format));

    if (v4l2_ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
        v4l2_ioctl(fd, VIDIOC_STREAMOFF, &buf.type);
        throw std::runtime_error("Could not queue buffer back: " + std::string(strerror(errno)));
    }

    if (v4l2_ioctl(fd, VIDIOC_STREAMOFF, &buf.type) < 0) {
        throw std::runtime_error("Could not turn off stream: " + std::string(strerror(errno)));
    }

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

std::ostream& operator<<(std::ostream& stream, const ImageFormat& format) {
    stream << "Format: " << format.fourcc << ", Width: " << format.width << ", Height: " << format.height;
    return stream;
}
