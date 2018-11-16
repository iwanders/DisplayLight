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
#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <vector>

#include <X11/Xresource.h>

/**
 * @brief This image class is either backed by an XImage or by a bitmap of uint32's.
 *        If written to, it is converted to a bitmap, it's size is immutable.
 */
class Image
{
  size_t width_;   //!< Width of the image.
  size_t height_;  //!< Height of the image.

  bool shared_memory_{ false };  //!< True if it uses the XImage, false if it uses the bitmap.

  // bitmap backend
  std::vector<std::vector<uint32_t>> map_;  //!< Bitmap representation of the data.

  // shared memory backend.
  std::shared_ptr<XImage> ximage_;  //!< XImage represents the data.

  Image() = default;

public:
  using Bitmap = std::vector<std::vector<uint32_t>>;

  /**
   * @brief Construct a Image from an XImage.
   */
  Image(std::shared_ptr<XImage> image);

  /**
   * @brief Constructs a Image from a bitmap.
   */
  Image(Bitmap map);

  /**
   * @brief Ensure a bitmap is used as data storage.
   */
  void convertToBitmap();

  /**
   * @brief Return the width of the image.
   */
  size_t getWidth() const;

  /**
   * @brief Return the height of the image.
   */
  size_t getHeight() const;

  /**
   * @brief Return the value of a pixel on the image. Format is 0x00RRGGBB
   */
  uint32_t pixel(size_t x, size_t y) const;

  /**
   * @brief Writes a certain value to a position.
   */
  void setPixel(size_t x, size_t y, uint32_t color);

  /**
   * @brief Create a vertical line on the image with the provided color.
   */
  void hLine(size_t y, uint32_t color);

  /**
   * @brief Create a horizontal line on the image with the provided color.
   */
  void vLine(size_t x, uint32_t color);

  /**
   * @brief Get the ppm presentation of the data in this image.
   */
  std::string imageToPPM();

  /**
   * @brief Write contants to a file in binary format.
   * Format is:
   * struct
   * {
   *   uint32_t width;
   *   uint32_t height;
   *   uint32_t data[width*height]; // Order; data[y][x].
   * }
   */
  void writeContents(const std::string& filename);

  /**
   * @brief Constructs a image from a binary file on the disk.
   */
  static Image readContents(const std::string& filename);
};

#endif