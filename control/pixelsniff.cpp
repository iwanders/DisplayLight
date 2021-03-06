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
#include "pixelsniff.h"
#include <chrono>
#include <fstream>
#include <sstream>

PixelSniffer::PixelSniffer()
{
}

void PixelSniffer::connect()
{
}

bool PixelSniffer::selectRootWindow()
{
  std::cout << "Pixelsniffer root" << std::endl;
  return prepareCapture();  // default to entire screen.
}

bool PixelSniffer::prepareCapture(size_t x, size_t y, size_t width, size_t height)
{
  std::cout << "Pixelsniffer prepareCapture" << std::endl;
  return false;
}

bool PixelSniffer::grabContent()
{
  std::cout << "Pixelsniffer grabContent" << std::endl;
  return false;
}

Image::Ptr PixelSniffer::getScreen()
{
  return std::make_shared<Image>(Image::Bitmap{});
}


PixelSniffer::Resolution PixelSniffer::getFullResolution()
{
  return Resolution(0, 0);
}

