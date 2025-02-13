#include <gtest/gtest.h>
#include "cameramanager.hpp"

TEST(checkDevices, AtLeastOneCameraPresent) {
    CameraManager& manager = CameraManager::getInstance();
    int count = manager.getNumberOfInputDevices();
    EXPECT_GE(manager.getNumberOfInputDevices(), 1) << "At least one camera is required";
}