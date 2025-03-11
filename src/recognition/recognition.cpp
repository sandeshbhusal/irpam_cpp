#include "include/recognition.hpp"
#include <stdexcept>
#include <random>
#include <iostream>
#include "image.hpp"
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION

extern "C" void recognize_image(const ImageBuffer& image)
{
    // save original image to a file.
    stbi_write_jpg("original_image.jpg", image.getFormat().width, image.getFormat().height, 3, image.getData(), 100);
 
    ImageBuffer resized_image = *image.resizeTo(320, 240);

    // save resized image to a file.
    stbi_write_jpg("resized_image.jpg", resized_image.getFormat().width, resized_image.getFormat().height, 3, resized_image.getData(), 100);

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

    // Grab the largest face detected in the model.
    int max_index = 0;
    float max_value = 0.0f;

    for (int i = 0; i < output0.h; ++i)
    {
        float value = output0.row(i)[0];
        if (value > max_value)
        {
            max_value = value;
            max_index = i;
        }
    }

    std::cerr << "Max value: " << max_value << " at index: " << max_index << std::endl;

    // Grab the bounding box for the face.
    float x0 = output1.row(max_index)[0];
    float y0 = output1.row(max_index)[1];
    float x1 = output1.row(max_index)[2];
    float y1 = output1.row(max_index)[3];

    std::cerr << "Bounding box: (" << x0 << ", " << y0 << ") (" << x1 << ", " << y1 << ")" << std::endl;

    // Crop the image to the bounding box.
    std::unique_ptr<ImageBuffer> cropped_image = resized_image.cropImage(x0, y0, x1, y1);
    std::cerr << "Cropped image size: " << cropped_image->getFormat().width << "x" << cropped_image->getFormat().height << std::endl;

    // Save the cropped image to a file (for testing right now)
    stbi_write_jpg("cropped_image.jpg", cropped_image->getFormat().width, cropped_image->getFormat().height, 3, cropped_image->getData(), 100);

    // Now we load a different neural net to generate facial embeddings.
    ncnn::Net embeddingsNet;
    // embeddingsNet.load_model();
    // embeddingsNet.load_param();
}