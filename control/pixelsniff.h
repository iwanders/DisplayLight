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
#ifndef PIXELSNIFF_H
#define PIXELSNIFF_H

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "image.h"

class PixelSniffer
{
public:
  PixelSniffer();

  /**
   * @brief Connect the X context, get display pointer. Check if the shared memory extension is present.
   */
  virtual void connect();

  /**
   * @brief Select the root window to capture from.
   * @return False if an error occured.
   */
  virtual bool selectRootWindow();

  /**
   * @brief Grab a snapshot of the capture area.
   */
  virtual bool grabContent() const;

  /**
   * @brief Return a Image instance that is backed by the current image in the pixelsniffer.
   * @return A screen backed by the shared XImage in this class.
   */
  virtual Image getScreen() const;

  /**
   * @brief Prepares the capture area in the window.
   * @param x The x coordinate in the window (starts left)
   * @param y The y coordinate in the window (starts top)
   * @param width The width of segment to receive, if 0 the window width if used.
   * @param height The height of segment to receive, if 0 the window height if used.
   */
  virtual bool prepareCapture(size_t x = 0, size_t y = 0, size_t width = 0, size_t height = 0);
};

#endif
