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

#include "image.hpp"

/**
 * @brief Video Device Entry in the system. The path is ensured to exist.
 */
class VideoDevice
{
private:
    std::string camera_path;
    std::vector<ImageFormat> available_formats;
    v4l2_capability cap;
    bool is_ir = false;
    bool supports_mmap = false;
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