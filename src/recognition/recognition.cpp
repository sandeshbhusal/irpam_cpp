#include "recognition.hpp"

std::optional<cv::Mat> extract_face(const cv::Mat &input_image)
{
    using namespace recognition_params;

    int image_width = input_image.size[1];
    int image_height = input_image.size[0];

    // Prepare input to NN
    cv::Mat resized_image;
    cv::resize(input_image, resized_image, cv::Size(DETECTION_NET_WIDTH, DETECTION_NET_WIDTH), 1.0);
    auto blob = cv::dnn::blobFromImage(resized_image);

    // Prep + run NN
    auto detection_model = cv::dnn::readNetFromCaffe(
        "/home/sandesh/workspace/opencv-testing/modelproto.txt",
        "/home/sandesh/workspace/opencv-testing/res10_300x300_ssd_iter_140000_fp16.caffemodel");

    detection_model.setInput(blob);
    auto detections = detection_model.forward();

    std::vector<DetectedFace> faces;
    for (int i = 0; i < detections.size[2]; ++i)
    {
        float confidence = detections.ptr<float>(0, 0, i)[2];
        int x = static_cast<int>(detections.ptr<float>(0, 0, i)[3] * image_width);
        int y = static_cast<int>(detections.ptr<float>(0, 0, i)[4] * image_height);
        int endx = static_cast<int>(detections.ptr<float>(0, 0, i)[5] * image_width);
        int endy = static_cast<int>(detections.ptr<float>(0, 0, i)[6] * image_height);

        if (confidence > 0.8)
            faces.push_back(DetectedFace{
                .x = x,
                .y = y,
                .w = endx - x,
                .h = endy - y,
                .size = (endx - x) * (endy - y),
                .confidence = confidence});
    }

    std::sort(faces.begin(), faces.end(), [](DetectedFace face1, DetectedFace face2)
              { return face1.size > face2.size; });

    // Get + return the largest face we could find.
    if (faces.empty())
    {
        return std::nullopt;
    }
    else
    {
        auto face = faces[0];
        int x = std::max(0, face.x);
        int y = std::max(0, face.y);
        int w = std::min(input_image.cols - face.x, face.w);
        int h = std::min(input_image.rows - face.y, face.h);

        cv::Rect roi(x, y, w, h);
        cv::Mat cropped = input_image(roi).clone();

        return cropped;
    }
}

cv::Mat get_embedding(const cv::Mat &image)
{
    using namespace recognition_params;
    auto net = cv::dnn::readNetFromONNX("/home/sandesh/workspace/opencv-testing/arcfaceresnet100-11-int8.onnx");
    net.setInput(image);
    return net.forward(OUTPUT_LAYER);
}

bool are_similar(const cv::Mat &first, const cv::Mat &second)
{
    using namespace recognition_params;
    auto blob1 = cv::dnn::blobFromImage(first, 1.0 / 128.0, cv::Size(EMBEDDING_NET_WIDTH, EMBEDDING_NET_WIDTH));
    auto blob2 = cv::dnn::blobFromImage(second, 1.0 / 128.0, cv::Size(EMBEDDING_NET_WIDTH, EMBEDDING_NET_WIDTH));

    auto embedding1 = get_embedding(blob1);
    auto embedding2 = get_embedding(blob2);

    auto dot_product = embedding1.dot(embedding2);
    auto norm1 = cv::norm(embedding1);
    auto norm2 = cv::norm(embedding2);

    auto cosine_similarity = dot_product / (norm1 * norm2);
    return cosine_similarity >= face_threshold;
}