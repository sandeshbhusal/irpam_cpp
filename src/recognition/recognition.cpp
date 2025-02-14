#include "include/recognition.hpp"
#include <stdexcept>

extern "C" void recognize_image(void *imageData, int width, int height)
{
    ncnn::Net network;
    // TODO: Here, the embedded resource is to be read.
    if (network.load_model((const char*) nullptr) < 1) {
        throw std::runtime_error("Failed to load the model from the file.");
    }
}