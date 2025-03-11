#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE2_IMPLEMENTATION

#include "image.hpp"
#include <cassert>

#include "stb_image_resize2.h"

ImageBuffer::ImageBuffer(const void *databuffer, uint32_t size, const ImageFormat format)
    : format(format), bufferSize(size), buffer(std::make_unique<char[]>(size))
{
    std::memcpy(buffer.get(), databuffer, size);
}

ImageBuffer::ImageBuffer(const ImageBuffer &other)
    : format(other.format), bufferSize(other.bufferSize), buffer(std::make_unique<char[]>(other.bufferSize))
{
    std::memcpy(buffer.get(), other.buffer.get(), bufferSize);
}

ImageBuffer &ImageBuffer::operator=(const ImageBuffer &other)
{
    if (this != &other)
    {
        format = other.format;
        bufferSize = other.bufferSize;
        buffer = std::make_unique<char[]>(other.bufferSize);
        std::memcpy(buffer.get(), other.buffer.get(), bufferSize);
    }
    return *this;
}

ImageBuffer::ImageBuffer(ImageBuffer &&other) noexcept
    : format(other.format), bufferSize(other.bufferSize), buffer(std::move(other.buffer))
{
    other.bufferSize = 0;
}

const ImageFormat &ImageBuffer::getFormat() const { return format; }

const void *ImageBuffer::getData() const { return this->buffer.get(); }
size_t ImageBuffer::getSize() const { return this->bufferSize; }

std::unique_ptr<ImageBuffer> ImageBuffer::resizeTo(unsigned int newWidth, unsigned int newHeight) const
{
    int numChannels = 3;
    int inputStride = format.width * numChannels;
    int outputStride = newWidth * numChannels;

    void *resizedBuffer = new char[newWidth * newHeight * numChannels];

    stbir_resize_uint8_srgb(
        reinterpret_cast<const unsigned char *>(buffer.get()), format.width, format.height, inputStride,
        reinterpret_cast<unsigned char *>(resizedBuffer), newWidth, newHeight, outputStride,
        STBIR_RGB);

    return std::make_unique<ImageBuffer>(resizedBuffer, newWidth * newHeight * numChannels,
                                         ImageFormat{format.fourcc, newWidth, newHeight, newWidth * newHeight * numChannels});
}


std::unique_ptr<ImageBuffer> ImageBuffer::cropImage(double x0, double y0, double x1, double y1) const {
    unsigned int start_x = static_cast<unsigned int>(x0 * format.width);
    unsigned int start_y = static_cast<unsigned int>(y0 * format.height);
    unsigned int end_x = static_cast<unsigned int>(x1 * format.width);
    unsigned int end_y = static_cast<unsigned int>(y1 * format.height);

    unsigned int new_width = end_x - start_x;
    unsigned int new_height = end_y - start_y;

    assert(new_width > 0 && new_height > 0 && "Invalid crop dimensions");

    size_t new_size = new_width * new_height * 3; // Assuming 3 channels (RGB)
    void *croppedBuffer = new char[new_size];

    for (unsigned int y = 0; y < new_height; ++y) {
        unsigned int src_y = start_y + y;
        unsigned int dst_y = y;

        for (unsigned int x = 0; x < new_width; ++x) {
            unsigned int src_x = start_x + x;
            unsigned int dst_x = x;

            size_t src_index = (src_y * format.width + src_x) * 3;
            size_t dst_index = (dst_y * new_width + dst_x) * 3;

            std::memcpy(static_cast<char *>(croppedBuffer) + dst_index,
                        static_cast<char *>(buffer.get()) + src_index, 3);
        }
    }
    return std::make_unique<ImageBuffer>(croppedBuffer, new_size,
                                         ImageFormat{format.fourcc, new_width, new_height, new_size});
}