#include "image.hpp"

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
