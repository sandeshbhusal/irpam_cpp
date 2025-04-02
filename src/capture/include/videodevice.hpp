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
    int fd = -1;

public:
    explicit VideoDevice(const std::string &camera_path);

    /**
    * @brief Check if this device is a camera. There can be multiple types of video devices,
    * like metadata capture, video output, etc.
    */
    bool isCaptureDevice() const;

    /**
     * @brief Return the device path (e.g., /dev/video0).
     */
    const std::string getPath() const;

    /**
     * @brief Get available formats supported by this device.
     */
    const std::vector<ImageFormat> getAvailableFormats() const;

    /**
     * @brief Capture multiple images from the video device. This is required
     * in scenarios, like training the neural net for the first time.
     */
    std::vector<std::unique_ptr<ImageBuffer>> grab_multiple(const ImageFormat& format, int count) const;
};

std::vector<std::string> availableVideoDevices();
#endif