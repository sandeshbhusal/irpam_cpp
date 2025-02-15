#include <gtest/gtest.h>
#include "cameramanager.hpp"

TEST(checkDevices, AtLeastOneCameraPresent) {
    CameraManager& manager = CameraManager::getInstance();
    int count = manager.getNumberOfInputDevices();
    EXPECT_GE(manager.getNumberOfInputDevices(), 1) << "At least one camera is required";
}

TEST(checkCapture, AtLeastOneImage) {
    // Really doesn't matter what format we use here :)
    ImageFormat format = {0};

    CameraManager& manager = CameraManager::getInstance();
    
    // Get the first camera.
    std::shared_ptr<VideoDevice> device = manager.get_camera_from_index(0);
    std::unique_ptr<ImageBuffer> buffer = device.get()->grab(format);
}