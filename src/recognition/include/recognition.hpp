#ifndef RECOGNITION_H
#define RECOGNITION_H

#include "net.h"
#include "image.hpp"

extern "C" void recognize_image(const ImageBuffer &image);
#endif