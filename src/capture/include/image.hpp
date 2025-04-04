#ifndef IMAGE_H
#define IMAGE_H

#include <cstdint>
#include <cstring>
#include <memory>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include "libv4lconvert.h"

/**
 * @brief A supported image format in the camera device.
 * This struct only stores the pixel format fourcc and dimensions of the image.
 */
struct ImageFormat
{
    uint32_t fourcc;
    unsigned int width;
    unsigned int height;
    size_t buffersize;

    const ImageFormat &getFormat();
    const void *getData();
    size_t getSize() const;
};

class ImageBuffer
{
private:
    ImageFormat format;
    std::unique_ptr<char[]> buffer;
    size_t bufferSize;

public:
    ImageBuffer(const void *databuffer, uint32_t size, const ImageFormat format);
    ImageBuffer(const ImageBuffer &other);
    ImageBuffer &operator=(const ImageBuffer &other);
    ImageBuffer(ImageBuffer &&other) noexcept;
    ImageBuffer &operator=(ImageBuffer &&other) noexcept;
    ~ImageBuffer() = default;
    const ImageFormat &getFormat() const;
    const void *getData() const;
    size_t getSize() const;

    std::unique_ptr<ImageBuffer> resizeTo(unsigned int newWidth, unsigned int newHeight) const;
    std::unique_ptr<ImageBuffer> cropImage(double x0, double y0, double x1, double y1) const;
    cv::Mat to_mat();
};

std::ostream &operator<<(std::ostream &stream, const ImageFormat &format);
#endif