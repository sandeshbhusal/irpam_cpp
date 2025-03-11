#include <thread>
#include <gtest/gtest.h>
#include "cameramanager.hpp"
#include "stb_image_write.hpp"

TEST(checkDevices, AtLeastOneCameraPresent)
{
    CameraManager &manager = CameraManager::getInstance();
    int count = manager.getNumberOfInputDevices();
    EXPECT_GE(manager.getNumberOfInputDevices(), 1) << "At least one camera is required";
}

TEST(checkCapture, DoubleTrouble)
{
    // Initialize two threads so that we can capture two images at the same time.
    std::thread t1;
    std::thread t2;

    auto runnable1 = []()
    {
        CameraManager &manager = CameraManager::getInstance();
        std::shared_ptr<VideoDevice> camera = manager.get_camera_from_index(0);
        std::unique_ptr<ImageBuffer> buffer = camera.get()->grab({.fourcc = v4l2_fourcc('M', 'J', 'P', 'G'), .width = 640, .height = 480});

        stbi_write_jpg("test_jpg.jpg", buffer.get()->getFormat().width, buffer.get()->getFormat().height, 3, buffer.get()->getData(), 100);
    };

    auto runnable2 = []()
    {
        CameraManager &manager = CameraManager::getInstance();
        std::shared_ptr<VideoDevice> camera = manager.get_camera_from_index(1);
        std::unique_ptr<ImageBuffer> buffer = camera.get()->grab({.fourcc = v4l2_fourcc('G', 'R', 'E', 'Y'), .width = 400, .height = 400});

        stbi_write_jpg("test_luma.jpg", buffer.get()->getFormat().width, buffer.get()->getFormat().height, 3, buffer.get()->getData(), 100);
    };

    // Start the threads
    t1 = std::thread(runnable1);
    t2 = std::thread(runnable2);

    // Wait for the threads to finish
    t1.join();
    t2.join();
}