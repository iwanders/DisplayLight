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
#ifndef ImageWin_H
#define ImageWin_H

#include <memory>
#include <vector>

#include "image.h"
#include "pixelsniffWin.h"

/**
 * @brief This ImageWin class is either backed by an XImageWin or by a bitmap of uint32's.
 *        If written to, it is converted to a bitmap, it's size is immutable.
 */
class ImageWin : public Image
{
  bool shared_memory_{ false };  //!< True if it uses the XImageWin, false if it uses the bitmap.

  // shared memory backend.
  std::shared_ptr<ID3D11Texture2D> image_;
  D3D11_MAPPED_SUBRESOURCE mapped_;

  ImageWin() = default;

public:
  using Bitmap = std::vector<std::vector<uint32_t>>;

  /**
   * @brief Construct a ImageWin from an XImageWin.
   */
  ImageWin(std::shared_ptr<ID3D11Texture2D> ImageWin);

  ImageWin(Bitmap v);

  /**
   * @brief Ensure a bitmap is used as data storage.
   */
  void convertToBitmap();

  /**
   * @brief Return the value of a pixel on the ImageWin. Format is 0x00RRGGBB
   */
  uint32_t pixel(size_t x, size_t y) const;
};

#endif