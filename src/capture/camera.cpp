#include "include/camera.hpp"

std::vector<std::string> availableCameras() {
    namespace fs = std::filesystem;
    std::vector<std::string> cameras;
    fs::path cameraDir = "/dev";
    for (const auto& entry : fs::directory_iterator(cameraDir)) {
        if (entry.is_character_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.find("video") == 0) {
                cameras.push_back(entry.path().string());
            } 
        }
    }
    
    return cameras;
}