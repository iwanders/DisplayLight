
#include "image.h"
#include <vector>
#include <memory>
#include <sstream>
#include <fstream>

Image::Image(std::shared_ptr<XImage> image)
{
  ximage_ = image;
  shared_memory_ = true;
  width_ = ximage_->width;
  height_ = ximage_->height;
}

Image::Image(Bitmap map)
{
  map_ = map;
  shared_memory_ = false;
  width_ = map.front().size();
  height_ = map.size();
}

void Image::convertToBitmap()
{
  if (shared_memory_)
  {
    // Perform copy of the data into the map.
    const uint8_t* data = reinterpret_cast<const uint8_t*>(ximage_->data);
    const uint8_t stride = ximage_->bits_per_pixel / 8;

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
  if (shared_memory_)
  {
    const uint8_t* data = reinterpret_cast<const uint8_t*>(ximage_->data);
    const uint8_t stride = ximage_->bits_per_pixel / 8;
    return (*reinterpret_cast<const uint32_t*>(data + y * width_ * stride + x * stride)) & 0x00FFFFFF;
  }
  else
  {
    return map_[y][x];
  }
}

void Image::setPixel(size_t x, size_t y, uint32_t color)
{
  convertToBitmap();
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


std::string Image::imageToPPM()
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
      const int blue = (color)&0xFF;
      ss << "" << red << " "
         << " " << green << " " << blue << " ";
    }
    ss << "\n";
  }
  return ss.str();
}

Image Image::readContents(const std::string& filename)
{
  std::ifstream ifs(filename, std::ios::binary|std::ios::ate);

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

  // Construct an image from this bitmap.
  return Image{contents};
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
