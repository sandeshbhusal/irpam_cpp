#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <errno.h>
#include <fcntl.h>
#include <cstdint>
#include <cstring>
#include <memory>

#include "libv4l2.h"
#include "libv4lconvert.h"
#include <string.h>

/**
 * @brief A supported image format in the camera device.
 * This struct only stores the pixel format fourcc and dimensions of the image.
 */
struct ImageFormat {
    uint32_t fourcc;
    unsigned int width;
    unsigned int height;
    size_t buffersize;
};

class ImageBuffer {
private:
    ImageFormat format;
    std::unique_ptr<char[]> buffer;
    size_t bufferSize;

public:
    ImageBuffer(const void* databuffer, uint32_t size, const ImageFormat format)
        : format(format), bufferSize(size), buffer(std::make_unique<char[]>(size)) {
        std::memcpy(buffer.get(), databuffer, size);
    }

    ImageBuffer(const ImageBuffer& other)
        : format(other.format), bufferSize(other.bufferSize), buffer(std::make_unique<char[]>(other.bufferSize)) {
        std::memcpy(buffer.get(), other.buffer.get(), bufferSize);
    }

    ImageBuffer& operator=(const ImageBuffer& other) {
        if (this != &other) {
            format = other.format;
            bufferSize = other.bufferSize;
            buffer = std::make_unique<char[]>(other.bufferSize);
            std::memcpy(buffer.get(), other.buffer.get(), bufferSize);
        }
        return *this;
    }

    ImageBuffer(ImageBuffer&& other) noexcept
        : format(other.format), bufferSize(other.bufferSize), buffer(std::move(other.buffer)) {
        other.bufferSize = 0;
    }

    ImageBuffer& operator=(ImageBuffer&& other) noexcept {
        if (this != &other) {
            format = other.format;
            bufferSize = other.bufferSize;
            buffer = std::move(other.buffer);
            other.bufferSize = 0;
        }
        return *this;
    }

    ~ImageBuffer() = default;

    const ImageFormat& getFormat() const { return format; }
    const void* getData() const { return buffer.get(); }
    size_t getSize() const { return bufferSize; }
};

std::ostream& operator<<(std::ostream& stream, const ImageFormat& format);

/**
 * @brief Video Device Entry in the system. The path is ensured to exist.
 */
class VideoDevice
{
private:
    std::string camera_path;
    std::vector<ImageFormat> available_formats;
    v4l2_capability cap;
    int fd = -1;

public:
    explicit VideoDevice(const std::string &camera_path);
    bool isCaptureDevice() const;
    const std::string getPath() const;
    std::vector<v4l2_pix_format> getAvailableFormats() const;
    std::unique_ptr<ImageBuffer> grab(const ImageFormat&) const;
};


std::vector<std::string> availableVideoDevices();
#endif