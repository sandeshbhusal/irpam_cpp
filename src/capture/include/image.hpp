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

/**
 * @brief ImageBuffer represents a collection of raw bytes that define an image
 * with a specific color space format and resolution, as described by ImageFormat.
 * 
 * ImageBuffer can be converted to a cv::Mat which supports a lot of image-based operations.
 * This class should be deprecated in the future in favor of cv::Mat, but for now, since
 * opencv fails to find IR cameras and we have to rely on low-level v4l operations to capture
 * images from IR camera onboard, this is here to stay.
 */
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

    /**
     * @brief Get the format of the image represented by this byte buffer.
     */
    const ImageFormat &getFormat() const;

    /**
     * @brief Returns a const ptr to the data in this image buffer. This data should be copied.
     */
    const void *getData() const;

    /**
     * @brief Returns the size of this byte buffer. This does **not** represent the image area (w x h).
     */
    size_t getSize() const;

    /**
     * @brief Copies the buffer and resizes it to produce another ImageBuffer.
     */
    std::unique_ptr<ImageBuffer> resizeTo(unsigned int newWidth, unsigned int newHeight) const;

    /**
     * @brief Crops the buffer to the specified ROI and produces another ImageBuffer.
     */
    std::unique_ptr<ImageBuffer> cropImage(double x0, double y0, double x1, double y1) const;

    /**
     * @brief Copies the buffer into a cv::Mat object.
     */
    cv::Mat to_mat() const;
};

std::ostream &operator<<(std::ostream &stream, const ImageFormat &format);
#endif