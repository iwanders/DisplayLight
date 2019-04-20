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
#include "image.h"
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>


Image::Image(Bitmap map)
{
  map_ = map;
  width_ = map.front().size();
  height_ = map.size();
}


void Image::convertToBitmap()
{
}

size_t Image::getWidth() const
{
  return width_;
}

size_t Image::getHeight() const
{
  return height_;
}

uint32_t Image::pixel(size_t x, size_t y) const
{
   return map_[y][x];
}

void Image::setPixel(size_t x, size_t y, uint32_t color)
{
  map_[y][x] = color;
}

void Image::hLine(size_t y, uint32_t color)
{
  for (size_t i = 0; i < getWidth(); i++)
  {
    setPixel(i, y, color);
  }
}

void Image::vLine(size_t x, uint32_t color)
{
  for (size_t i = 0; i < getHeight(); i++)
  {
    setPixel(x, i, color);
  }
}

std::string Image::imageToPPM() const
{
  std::stringstream ss;
  ss << "P3\n";
  ss << getWidth() << " " << getHeight() << "\n";
  ss << "255\n";
  ss << "# data now\n";
  for (size_t y = 0; y < getHeight(); y++)
  {
    for (size_t x = 0; x < getWidth(); x++)
    {
      uint32_t color = pixel(x, y);
      const int red = (color >> 16) & 0xFF;
      const int green = (color >> 8) & 0xFF;
      const int blue = (color) & 0xFF;
      ss << "" << red << " "
        << " " << green << " " << blue << " ";
    }
    ss << "\n";
  }
  return ss.str();
}

Image Image::readContents(const std::string& filename)
{
  std::ifstream ifs(filename, std::ios::binary | std::ios::ate);

  size_t width = 0;
  size_t height = 0;
  Bitmap contents;

  ifs.seekg(0, std::ios::beg);

  // Read width
  ifs.read(reinterpret_cast<char*>(&width), sizeof(width));

  // Read height
  ifs.read(reinterpret_cast<char*>(&height), sizeof(height));

  // Resize the number of rows
  contents.resize(height);
  for (auto& row : contents)
  {
    // For each row, resize it and read the appropriate number of bytes into it.
    row.resize(width);
    ifs.read(reinterpret_cast<char*>(row.data()), width * sizeof(contents.front().front()));
  }
  ifs.close();

  // Construct an Image from this bitmap.
  return Image{ contents };
}

void Image::writeContents(const std::string& filename)
{
  // Copy to a bitmap first, this makes it more convenient to work with.
  convertToBitmap();
  std::ofstream fout;
  fout.open(filename, std::ios::binary | std::ios::out);

  size_t width = getWidth();
  size_t height = getHeight();

  // Write width and height.
  fout.write(reinterpret_cast<const char*>(&width), sizeof(width));
  fout.write(reinterpret_cast<const char*>(&height), sizeof(height));

  // Write each row.
  for (auto& row : map_)
  {
    fout.write(reinterpret_cast<const char*>(row.data()), width * sizeof(row.front()));
  }

  fout.close();
}
