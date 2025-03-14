#include <assert.h>
#include "cameramanager.hpp"

CameraManager::CameraManager()
{
    std::vector<std::string> cameras = availableVideoDevices();
    for (const auto &path : cameras)
    {
        auto dev = std::make_shared<VideoDevice>(path);
        if (dev->isCaptureDevice())
            this->devices.push_back(dev);
    }
}

int CameraManager::getNumberOfInputDevices()
{
    return this->devices.size();
}

std::shared_ptr<VideoDevice> CameraManager::get_camera_from_index(int index)
{
    if (index >= this->devices.size())
    {
        throw std::runtime_error("Camera index exceeds number of available cameras.");
    }
    return this->devices[index];
}

std::shared_ptr<VideoDevice> CameraManager::get_camera_from_path(const char* path) {
    for (const auto& device: this->devices) {
        if (device->getPath() == std::string(path)) {
            return device;
        }
    }

    throw std::runtime_error("No such camera found");
}

enum LumaFormats {
    GREY = V4L2_PIX_FMT_GREY
};

/**
 * Retuns all the cameras that are capable of capturing luma images.
 */
std::vector<std::shared_ptr<VideoDevice>> get_luma_cameras() {
    return std::vector<std::shared_ptr<VideoDevice>>(); 
}