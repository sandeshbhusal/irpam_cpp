#include <gtest/gtest.h>
#include "camera.hpp"

TEST(checkTest, getcapturedevices) {
    std::vector<std::string> cameras = availableCameras();
    std::vector<std::string> captureDevices;
    
    for (const auto& camera : cameras) {
        try {
            Camera cam(camera);
            
            if (cam.isCaptureDevice()) {
                captureDevices.push_back(camera);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error with " << camera << ": " << e.what() << std::endl;
        }
    }
    
    EXPECT_GE(captureDevices.size(), 1) << "At least one camera should be available";
}