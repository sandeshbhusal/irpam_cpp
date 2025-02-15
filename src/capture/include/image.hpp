#ifndef IMAGE_H
#define IMAGE_H

#include <cstdint>
#include <cstring>
#include <memory>

/**
 * @brief A supported image format in the camera device.
 * This struct only stores the pixel format fourcc and dimensions of the image.
 */
struct ImageFormat {
    uint32_t fourcc;
    unsigned int width;
    unsigned int height;
    size_t buffersize;

    const ImageFormat &getFormat();
    const void *getData();
    size_t getSize() const;
};


class ImageBuffer {
private:
    ImageFormat format;
    std::unique_ptr<char[]> buffer;
    size_t bufferSize;

public:
    ImageBuffer(const void* databuffer, uint32_t size, const ImageFormat format);
    ImageBuffer(const ImageBuffer& other);
    ImageBuffer& operator=(const ImageBuffer& other);
    ImageBuffer(ImageBuffer&& other) noexcept;
    ImageBuffer& operator=(ImageBuffer&& other) noexcept;
    ~ImageBuffer() = default;
    const ImageFormat& getFormat() const;
    const void* getData() const;
    size_t getSize() const;
};

#endif