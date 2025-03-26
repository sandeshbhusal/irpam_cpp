#include <gtest/gtest.h>
#include <vector>
#include "cameramanager.hpp"
#include "recognition.hpp"

TEST(recognition_tests, BasicMatMul)
{
    CameraManager& camera_manager = CameraManager::getInstance();
    std::shared_ptr<VideoDevice> device = camera_manager.get_camera_from_index(0);
    auto buffer = device.get()->grab({
        .fourcc = v4l2_fourcc('G', 'R', 'E', 'Y'),
        .width = 400,
        .height = 400
    });

    auto cv_image = (*buffer).to_mat();
    // Make sure it returns true for both same images.
    ASSERT_TRUE(are_similar(cv_image, cv_image));
}

TEST(ir_capture, IR) {
    CameraManager& camera_manager = CameraManager::getInstance();
    for (int i = 0; i < camera_manager.getNumberOfInputDevices(); i++) {
        std::shared_ptr<VideoDevice> device = camera_manager.get_camera_from_index(i);
        
    }
}