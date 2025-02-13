#include "include/cameramanager.hpp"

CameraManager::CameraManager() {
    std::vector<std::string> cameras = availableVideoDevices();
    std::vector<VideoDevice> captureDevices;
    int index = 0;
    for (const auto& path: cameras) {
        VideoDevice dev(path, index);
        if (dev.isCaptureDevice())
            captureDevices.push_back(dev);
        index++;
    }

    this->devices = captureDevices;
}

int CameraManager::getNumberOfInputDevices() {
    return this->devices.size();
}
