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
#include <memory>

#include "libv4l2.h"
#include <string.h>

/**
 * @brief Video Device Entry in the system. The path is ensured to exist.
 */
class VideoDevice
{
private:
    std::string camera_path;
    std::vector<v4l2_pix_format> available_formats;
    v4l2_capability cap;
    int index = 0;
    int fd = -1;

public:
    explicit VideoDevice(const std::string &camera_path, int index);
    bool isCaptureDevice() const;
    const std::string getPath() const;
    std::vector<v4l2_pix_format> getAvailableFormats() const;
};


std::vector<std::string> availableVideoDevices();
#endif