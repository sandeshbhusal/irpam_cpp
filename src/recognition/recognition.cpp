#include "include/recognition.hpp"
#include <stdexcept>
#include <random>
#include <iostream>
#include "image.hpp"

extern "C" void recognize_image()
{
    // Create a new image buffer we can mutate.
    // ImageBuffer buffer(image);

    ncnn::Net network;
    network.load_param("/home/sandesh/temp/version_RFB_320.ncnn.param");
    network.load_model("/home/sandesh/temp/version_RFB_320.ncnn.bin");

    auto ex = network.create_extractor();

    // Let's try to input some nonsense that fits the size.
    std::vector<float> input_data(240*320*3);
    for (int i = 0; i < 240*320*3; ++i)
    {
        input_data[i] = random() / (random() + 1.0f );
    }

    for (const auto& layer: network.layers()) {
        std::cerr << "Layer: " << layer->name;
        std::cerr << " " << layer->type << std::endl;
    }

    ex.input("in0", ncnn::Mat(240, 320, 3, input_data.data()));

    // There are two output layers in this model.
    ncnn::Mat output0;
    ex.extract("out0", output0);
    ncnn::Mat output1;
    ex.extract("out1", output1);

    std::cerr << "Output0: " << output0.w << "x" << output0.h << "x" << output0.c << std::endl;
    std::cerr << "Output1: " << output1.w << "x" << output1.h << "x" << output1.c << std::endl;
}