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
#ifndef ANALYZER_H
#define ANALYZER_H

#include <cstdint>
#include <functional>
#include <sstream>
#include <vector>
#include "box.h"
#include "image.h"
#include "lights.h"

/**
 * @brief This represents a set of sample coordinates and the ledbox associated to it.
 */
struct BoxSamples
{
  Box box;
  std::vector<std::pair<size_t, size_t>> points;
};

/**
 * @brief Class that can perform analysis of the screen to come to the colors that can be sent to the LED's.
 * General approach consists of three steps:
 *    1. Use findBorders(content, ....); to find the bounds of the black borders.
 *    2. Call makeBoxedSamplePoints(dist, ....) with those bounds to make the boxed samples.
 *    3. Call sampleBoxSamples( ... ) to use those boxed samples to actually sample.
 * These steps are seperate because it allows caching step 2, which takes about 1/3 of the time (100 usec).
 */
class Analyzer
{
  size_t horizontal_celldepth_{ 200 };  //!< The depth of the cells in horizontal direction.
  size_t vertical_celldepth_{ 200 };    //!< The depth of led cells in vertical direction.

public:
  /**
   * @brief Make a canvas of the appropriate size.
   */
  static std::vector<RGB> makeCanvas();

  /**
   * @brief Set the depth of the horizontal and vertical cells. This is the distance they protrude into the screen from
   *        the border.
   */
  void setCellDepth(size_t horizontal, size_t vertical);

  /**
   * @brief Perform bisection procedures to find the left, bottom, top and right borders.
   * @param image The image to perform the bisection on.
   * @param bisects_per_side Default 4, the number of bisections to do for each side, these are equidistant to borders
   *        and themselves. This makes the system more robust against a black center pixel.
   * @return The bounding box of the region of interest.
   */
  Box findBorders(const Image& image, size_t bisects_per_side = 4) const;

  /**
   * @brief This computes a list of boxes for the given bounds and calculates the position of the sample points inside
   *        each box.
   * @param dist_between_samples The distance between samples in both horizontal and vertical direction (inside the box)
   * @param bounds The bounds of the entire region that will be analyzed, as created by findBorders.
   */
  std::vector<BoxSamples> makeBoxSamples(const size_t dist_between_samples, const Box& bounds);

  /**
   * @brief Sample a screen, given the screen the start position of the region to analyze, the precompued boxed samples
   *        and output the canvas of led colors.
   * @param screen The screen as captured by a capture device (the pixelsniffer).
   * @param bounds The bounds used to create the boxed samples.
   * @param boxed_samples The precomputed boxed samples, these points will be sampled for each box.
   * @param canvas The output vector of led colors.
   */
  void sample(const Image& screen, const Box& bounds, const std::vector<BoxSamples>& boxed_samples,
              std::vector<RGB>& canvas);

  /**
   * @brief Colorize a screen based on the colors in the canvas. This creates boxes on the edge that are 50 pixels deep.
   * @param canvas The canvas to draw on the screen.
   * @param[in, out] Outer borders of the image will get the boxes drawn on them.
   */
  void boxColorizer(const std::vector<RGB>& canvas, Image& image);
};
#endif
