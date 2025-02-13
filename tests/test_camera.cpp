#include <gtest/gtest.h>
#include "camera.hpp"

TEST(checkDevices, AtLeastOneCameraPresent) {
    CameraManager *manager = getCameraManager();
    int count = manager->getNumberOfInputDevices();
    EXPECT_GE(manager->getNumberOfInputDevices(), 1) << "At least one camera is required";
}