#include "include/recognition.hpp"
#include <stdexcept>
#include "image.hpp"

extern "C" void recognize_image(const ImageBuffer& image, int width, int height)
{
    // Create a new image buffer we can mutate.
    ImageBuffer buffer(image);

    ncnn::Net network;
    // TODO: Here, the embedded resource is to be read.
    if (network.load_model((const char*) nullptr) < 1) {
        throw std::runtime_error("Failed to load the model from the file.");
    }
}