/*
  The MIT License (MIT)
  Copyright (c) 2019 Ivor Wanders
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
#include "imageWin.h"
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

ImageWin::ImageWin(std::shared_ptr<ID3D11Texture2D> image)
{
  // Store the image, set to shared memory usage.
  image_ = image;
  shared_memory_ = true;

  D3D11_TEXTURE2D_DESC desc;
  image_->GetDesc(&desc);
  width_ = desc.Width;
  height_ = desc.Height;

  // Map the texture, retrieval of device and context and mapping from.
  // https://github.com/Microsoft/graphics-driver-samples/blob/master/render-only-sample/rostest/util.cpp

  // Retrieve device pointer.
  ID3D11Device* device;
  image_->GetDevice(&device);
  auto dev_clean = releasing(device);

  // Retrieve device context.
  ID3D11DeviceContext* context;
  device->GetImmediateContext(&context);
  auto context_clean = releasing(context);

  // Finally, map the result.
  HRESULT hr = context->Map(image_.get(),
                            0,  // Subresource
                            D3D11_MAP_READ,
                            0,  // MapFlags
                            &mapped_);
  // Ignore any response.
}

ImageWin::ImageWin(Bitmap map) : Image(map)
{
  // made a bitmap through this constructor...
  shared_memory_ = false;
}

void ImageWin::convertToBitmap()
{
  if (shared_memory_)
  {
    // convert this image into a bitmap.
    auto res = Image::Bitmap{};
    for (size_t y = 0; y < getHeight(); y++)
    {
      res.push_back({});
      for (size_t x = 0; x < getWidth(); x++)
      {
        auto z = reinterpret_cast<std::uint8_t*>(mapped_.pData);
        const uint8_t stride = (mapped_.RowPitch / getWidth());
        z = &(z[y * mapped_.RowPitch + x * stride]);
        res.back().push_back(*reinterpret_cast<std::uint32_t*>(z));
      }
    }
    map_ = res;
    shared_memory_ = false;
  }
}

uint32_t ImageWin::pixel(size_t x, size_t y) const
{
  if (shared_memory_)
  {
    // Cool, look into the mapped data.
    const uint8_t* data = reinterpret_cast<const uint8_t*>(mapped_.pData);
    const uint8_t stride = (mapped_.RowPitch / getWidth());
    return (*reinterpret_cast<const uint32_t*>(data + y * mapped_.RowPitch + x * stride)) & 0x00FFFFFF;
  }
  else
  {
    return Image::pixel(x, y);
  }
}
