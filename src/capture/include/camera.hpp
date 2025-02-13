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

#include "libv4l2.h"
#include <string.h>

/**
 * @brief PictureFormat structure to hold the format of the camera.
 */
struct PictureFormat
{
    uint32_t fourcc;
    int width;
    int height;
    int channels;
};

/**
 * @brief Camera Definition Entry in the system. The path is ensure to exist.
 */
class Camera
{
private:
    std::string camera_path;
    v4l2_capability cap{}; // Initialize to zero
    int fd = -1;

public:
    ~Camera()
    {
        if (this->fd > -1)
            close(this->fd);
    }

    explicit Camera(const std::string &camera_path)
        : camera_path(camera_path)
    {
        if (!std::filesystem::exists(camera_path))
            throw std::runtime_error("Camera path does not exist: " + camera_path);

        this->fd = v4l2_open(camera_path.c_str(), O_RDWR);
        if (this->fd < 0)
            throw std::runtime_error("Failed to open camera: " + std::string(strerror(errno)));

        if (v4l2_ioctl(fd, VIDIOC_QUERYCAP, &this->cap) < 0)
            throw std::runtime_error("Failed to query camera caps: " + std::string(strerror(errno)));
    }

    bool isCaptureDevice() const
    {
        int input = 0;
        if (v4l2_ioctl(fd, VIDIOC_G_INPUT, &input) < 0)
            return false;
        
        return true;
    }

    const std::string getCameraPath() const
    {
        return this->camera_path;
    }
};

std::vector<std::string> availableCameras();

#endif