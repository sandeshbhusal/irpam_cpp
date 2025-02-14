#ifndef VIDEO_DEVICE_H
#define VIDEO_DEVICE_H
#include <memory>
#include <vector>
#include "videodevice.hpp"

/**
 * @brief Camera Manager interface. This is responsible to initialize all cameras
 * in the system, open them and keep a reference around for the lifetime of the program.
 * This is a singleton object, i.e. it will be initialized only once throughout the lifetime
 * of the program.
 */
class CameraManager
{
private:
    std::vector<std::shared_ptr<VideoDevice*>> devices;
    CameraManager();

public:
    CameraManager(const CameraManager &) = delete;
    CameraManager &operator=(const CameraManager &) = delete;

    static CameraManager &getInstance()
    {
        static CameraManager instance;
        return instance;
    }

    std::shared_ptr<VideoDevice*> get_camera_from_index(int);
    std::shared_ptr<VideoDevice*> get_camera_from_path(const char *);

    int getNumberOfInputDevices();
};
#endif