#include "include/camera.hpp"

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

VideoDevice::VideoDevice(const std::string &camera_path, int index)
    : camera_path(camera_path), index(index)
{
    if (!std::filesystem::exists(camera_path))
        throw std::runtime_error("Camera path does not exist: " + camera_path);

    this->fd = v4l2_open(camera_path.c_str(), O_RDWR);
    if (this->fd < 0)
        throw std::runtime_error("Failed to open camera: " + std::string(strerror(errno)));

    if (v4l2_ioctl(fd, VIDIOC_QUERYCAP, &this->cap) < 0)
        throw std::runtime_error("Failed to query camera caps: " + std::string(strerror(errno)));
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

CameraManager* getCameraManager() {
    static CameraManager* ptr = nullptr;
    if (ptr == nullptr) {
        // initialize.
        ptr = new CameraManager();
        return ptr;
    } else {
        return ptr;
    }
}