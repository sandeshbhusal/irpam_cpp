#ifndef RECOGNITION_H
#define RECOGNITION_H

#include <stdexcept>
#include <random>

#include "recognition.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/dnn.hpp"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <optional>

/**
 * Splitting params into a namespace because of header-clash issues
 * I do not know how to fix yet (or have the bandwidth to fix right now)
 */
namespace recognition_params
{
    const int DETECTION_NET_WIDTH = 300;
    const int EMBEDDING_NET_WIDTH = 112;
    const int EMBEDDING_WIDTH = 512;
    const char *OUTPUT_LAYER = "fc1";

    // Params for our model. These need to be put inside
    // `/etc/portapam/
    const float face_threshold = 0.85;
    const float similarity_threshold = 0.85;
}
struct DetectedFace
{
    int x;
    int y;
    int w;
    int h;
    int size;
    float confidence;
};

/**
 * @brief
 */
std::optional<cv::Mat> extract_face(const cv::Mat &input_image);
cv::Mat get_embedding(const cv::Mat &image);
bool are_similar(const cv::Mat &first, const cv::Mat &second);
#endif