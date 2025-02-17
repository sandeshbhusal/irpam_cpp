// #define STB_IMAGE_RESIZE_IMPLEMENTATION
// #define STB_IMAGE_RESIZE2_IMPLEMENTATION

// #include "stb_image_resize2.hpp"
#include "image.hpp"
#include <cassert>

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

void ImageBuffer::resizeBuffer(size_t newWidth, size_t newHeight)
{
    // Check if nothing to do
    if (newWidth == this->format.width && newHeight == this->format.height)
        return;

    // Assert that our format is indeed in RGB.
    assert(this->format.fourcc == V4L2_PIX_FMT_RGB24 && "ImageBuffer::resizeBuffer: format is not RGB24");

    // Calculate the new size
    size_t newSize = newWidth * newHeight * 3; // 3 bytes per pixel for RGB24
    auto newBuffer = std::make_unique<char[]>(newSize);

    // stbir_resize_uint8_srgb(
    //     reinterpret_cast<const unsigned char *>(this->buffer.get()), this->format.width, this->format.height,
    //     0, reinterpret_cast<unsigned char *>(newBuffer.get()), newWidth, newHeight, 0, STBIR_RGB);

    // // Update the buffer and format
    // this->buffer = std::move(newBuffer);
    // this->bufferSize = newSize;
    // this->format.width = newWidth;
    // this->format.height = newHeight;
}

const ImageFormat &ImageBuffer::getFormat() const { return format; }

const void *ImageBuffer::getData() const { return this->buffer.get(); }
size_t ImageBuffer::getSize() const { return this->bufferSize; }
