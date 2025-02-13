#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <fcntl.h>

#include "libv4l2.h"

/**
 * @brief PictureFormat structure to hold the format of the camera.
 */
struct PictureFormat {
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
    std::string card_name;
    std::string driver_name;
    bool is_capture_device = false;
    std::vector<PictureFormat> formats;

public:
    explicit Camera(const std::string &camera_path)
        : camera_path(camera_path)
    {
        // Ensure the camera exists.
        if (!std::filesystem::exists(camera_path))
            throw std::runtime_error("Camera path does not exist: " + camera_path);
        
        // Query the device for its formats.
        v4l2_capability cap;
        int fd = v4l2_open(camera_path.c_str(), O_RDWR);
        if (fd < 0)
            throw std::runtime_error("Failed to open camera: " + camera_path);
        
        if (v4l2_ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
            throw std::runtime_error("Failed to query camera capabilities: " + camera_path); 

        // Ensure this is a video capture device. If not, mark it as a video output device.
        this -> is_capture_device = (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) != 0;
        this -> card_name = std::string(reinterpret_cast<char*>(cap.card), sizeof(cap.card));
        this -> driver_name = std::string(reinterpret_cast<char*>(cap.driver), sizeof(cap.driver));

        close(fd);
        
        // If this is an output device, we are done at this point.        
        if (!this->is_capture_device) return;
    }

    bool isCaptureDevice() {
        return false;
    }

    std::string getCameraPath() const {
        return camera_path;
    }
};

std::vector<std::string> availableCameras();

#endif