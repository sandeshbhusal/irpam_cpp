#ifndef RECOGNITION_H
#define RECOGNITION_H

#include <stdexcept>
#include <random>

#include "recognition.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/dnn.hpp"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <optional>

// We need to split these out from the def 
// because we have not settled on a single model yet,
// and these are the params that keep changing between models.
const int DETECTION_NET_WIDTH = 300;
const int EMBEDDING_NET_WIDTH = 112;
const int EMBEDDING_WIDTH = 512;

// Params for our model. These need to be put inside
// `/etc/portapam/
const float face_threshold = 0.85;
const float similarity_threshold = 0.85;

/**
 * @brief A detected face. This is used to store a position of a face among multiple
 * faces found in an image.
 */
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
 * @brief Extract a face (if exists) from the input image.
 * 
 * @param input_image The input image.
 * @return std::optional<cv::Mat> The face if found.
 */
std::optional<cv::Mat> extract_face(const cv::Mat &input_image);

/**
 * @brief Get the embedding of the input image. The embedding is 512-floats vector.
 * 
 * @param image The input image.
 * @return cv::Mat The embedding.
 */
cv::Mat get_embedding(const cv::Mat &image);

/**
 * @brief Compare two images and return if they are similar. This computes the 
 * embeddings internally and compares the cosine similarity.
 * 
 * @param first The first image.
 * @param second The second image.
 * @return true If they are similar.
 */
bool are_similar(const cv::Mat &first, const cv::Mat &second);
#endif