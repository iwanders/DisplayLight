/*
  The MIT License (MIT)
  Copyright (c) 2018 Ivor Wanders
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include "ImageX11.h"
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

ImageX11::ImageX11(std::shared_ptr<XImageX11> ImageX11)
{
  xImageX11_ = ImageX11;
  shared_memory_ = true;
  width_ = xImageX11_->width;
  height_ = xImageX11_->height;
}

ImageX11::ImageX11(Bitmap map)
{
  map_ = map;
  shared_memory_ = false;
  width_ = map.front().size();
  height_ = map.size();
}

void ImageX11::convertToBitmap()
{
  if (shared_memory_)
  {
    // Perform copy of the data into the map.
    const uint8_t* data = reinterpret_cast<const uint8_t*>(xImageX11_->data);
    const uint8_t stride = xImageX11_->bits_per_pixel / 8;

    map_.resize(height_);
    for (size_t y = 0; y < height_; y++)
    {
      map_[y].resize(width_);
      for (size_t x = 0; x < width_; x++)
      {
        uint32_t value = *reinterpret_cast<const uint32_t*>(data + y * width_ * stride + x * stride);
        map_[y][x] = value & 0x00FFFFFF;
      }
    }
    shared_memory_ = false;
  }
}

size_t ImageX11::getWidth() const
{
  return width_;
}

size_t ImageX11::getHeight() const
{
  return height_;
}

uint32_t ImageX11::pixel(size_t x, size_t y) const
{
  if (shared_memory_)
  {
    const uint8_t* data = reinterpret_cast<const uint8_t*>(xImageX11_->data);
    const uint8_t stride = xImageX11_->bits_per_pixel / 8;
    return (*reinterpret_cast<const uint32_t*>(data + y * width_ * stride + x * stride)) & 0x00FFFFFF;
  }
  else
  {
    return map_[y][x];
  }
}

void ImageX11::setPixel(size_t x, size_t y, uint32_t color)
{
  convertToBitmap();
  map_[y][x] = color;
}

