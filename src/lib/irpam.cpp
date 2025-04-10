#include <filesystem>

#include <spdlog/spdlog.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

#include "cameramanager.hpp"
#include "recognition.hpp"
#include "include/irpam.hpp"

// Returns a list of config options that are missing.
std::vector<std::string> get_missing_config_options(std::vector<std::string> required_options,
                                                    const nlohmann::json &config)
{
    std::vector<std::string> missing_options;
    for (const auto &option : required_options)
    {
        if (config.find(option) == config.end())
        {
            missing_options.push_back(option);
        }
    }
    return missing_options;
}

extern "C" int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    // 1. Check if we have the /etc/irpam.json configuration file.
    std::ifstream config_file("/etc/irpam.json");
    if (!config_file.is_open())
    {
        std::cerr << "Configuration file /etc/irpam.json not found." << std::endl;
        return PAM_AUTH_ERR; // Configuration file not found
    }

    nlohmann::json config;
    try
    {
        config_file >> config;
    }
    catch (const nlohmann::json::parse_error &e)
    {
        std::cerr << "Error parsing JSON configuration: " << e.what() << std::endl;
        return PAM_AUTH_ERR; // JSON parsing error
    }

    // 2. Check for required configuration options
    std::vector<std::string> required_options = {
        "device_path",
        "fourcc",
        "image_width",
        "image_height",
        "buffersize",
        "detection_threshold"};

    auto missing_options = get_missing_config_options(required_options, config);
    if (!missing_options.empty())
    {
        // Throw an error and return pam auth failure.
        std::cerr << "Missing configuration parameters in /etc/irpam.json: ";
        for (const auto &option : missing_options)
        {
            std::cerr << option << " ";
        }
        std::cerr << std::endl;
        return PAM_AUTH_ERR;
    }

    // 3. Get configuration information, build image format.
    std::string device_path = config.value("device_path", "/dev/video0");
    std::string fourcc = config.value("fourcc", "GREY");
    int image_width = config.value("image_width", 400);
    int image_height = config.value("image_height", 400);
    int buffersize = config.value("buffersize", 480000);
    int detection_threshold = config.value("detection_threshold", 0.7);

    CameraManager &manager = CameraManager::getInstance();
    auto dev = manager.get_camera_from_path("/dev/video2").get();
    if (!dev)
    {
        std::cerr << "Video Device not found" << std::endl;
        return PAM_AUTH_ERR; // Camera not found
    }

    auto image_format = ImageFormat(
        fourcc.c_str(), // FourCC code
        image_width,    // Width
        image_height,   // Height
        buffersize      // Buffer size
    );

    // 4. Capture image with the configured format.
    auto images = dev->grab_multiple(image_format, 5);

    for (const auto &image : images)
    {
        auto imgptr = image.get();
        if (auto extracted_face_ptr = extract_face(imgptr->to_mat()))
        {
            auto extracted_face = *extracted_face_ptr;

            // Check the cosine distance from seen faces.
            const auto base_path = "/opt/irpam/data";
            double average_similarity = 0;
            int count = 0;

            if (!std::filesystem::exists(base_path) || !std::filesystem::is_directory(base_path))
            {
                std::cerr << "Directory does not exist or is not accessible: " << base_path << std::endl;
                return PAM_AUTH_ERR; // Directory not found or inaccessible
            }

            for (const auto &filepath : std::filesystem::directory_iterator(base_path))
            {
                std::cerr << filepath.path().string() << std::endl;
                if (filepath.is_regular_file())
                {
                    auto known_face = cv::imread(filepath.path().string());
                    if (known_face.empty())
                    {
                        std::cerr << "Failed to load image: " << filepath.path().string() << std::endl;
                        continue; // Skip this file if it cannot be loaded
                    }
                    double similarity = get_similarity(extracted_face, known_face);
                    average_similarity += similarity;
                    count++;
                }
            }

            if (count > 0)
            {
                average_similarity /= count;
                std::cerr << "Average similarity: " << average_similarity << std::endl;
                if (average_similarity >= 0.7) // Threshold for similarity
                {
                    return PAM_SUCCESS; // Authentication successful
                }
            }
            else
            {
                std::cerr << "No known faces to compare against. Cannot authenticate. Images:" << count << std::endl;
                return PAM_AUTH_ERR; // Not enough data to authenticate
            }
        }
    }
    std::cerr << "No face detected in captured images." << std::endl;
    return PAM_AUTH_ERR; // No face detected
}

extern "C" int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}

extern "C" int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}

extern "C" int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}
