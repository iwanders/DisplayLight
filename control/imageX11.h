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
#ifndef ImageX11_H
#define ImageX11_H

#include <memory>
#include <vector>

#include <image.h>
#include <X11/Xresource.h>

/**
 * @brief This ImageX11 class is either backed by an XImageX11 or by a bitmap of uint32's.
 *        If written to, it is converted to a bitmap, it's size is immutable.
 */
class ImageX11 : public Image
{
  bool shared_memory_{ false };  //!< True if it uses the XImageX11, false if it uses the bitmap.

  // shared memory backend.
  std::shared_ptr<XImageX11> xImageX11_;  //!< XImageX11 represents the data.

  ImageX11() = default;

public:
  using Bitmap = std::vector<std::vector<uint32_t>>;

  /**
   * @brief Construct a ImageX11 from an XImageX11.
   */
  ImageX11(std::shared_ptr<XImageX11> ImageX11);

  /**
   * @brief Ensure a bitmap is used as data storage.
   */
  void convertToBitmap();

  /**
   * @brief Return the width of the ImageX11.
   */
  size_t getWidth() const;

  /**
   * @brief Return the height of the ImageX11.
   */
  size_t getHeight() const;

  /**
   * @brief Return the value of a pixel on the ImageX11. Format is 0x00RRGGBB
   */
  uint32_t pixel(size_t x, size_t y) const;

  /**
   * @brief Writes a certain value to a position.
   */
  void setPixel(size_t x, size_t y, uint32_t color);

  /**
   * @brief Create a vertical line on the ImageX11 with the provided color.
   */
  void hLine(size_t y, uint32_t color);

  /**
   * @brief Create a horizontal line on the ImageX11 with the provided color.
   */
  void vLine(size_t x, uint32_t color);

};

#endif