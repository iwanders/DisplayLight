#ifndef ANALYZER_H
#define ANALYZER_H

#include <vector>
#include <functional>
#include <cstdint>
#include <sstream>
#include "../firmware/messages.h"
#include "image.h"

/**
 * @brief A rectangle.
 */
struct Box
{
  size_t x_min { 0 };  //!< The left bound.
  size_t x_max { 0 };  //!< The right bound.
  size_t y_min { 0 };  //!< The top bound.
  size_t y_max { 0 };  //!< The bottom bound.

  Box() = default;
  Box(size_t xmin, size_t xmax, size_t ymin, size_t ymax);

  operator std::string() const;
  bool operator <(const Box &b) const;

  size_t width() const;
  size_t height() const;
};

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
 * These steps are seperate because it allows caching stap 2, which takes about 1/3 of the time (100 usec).
 *       
 * @note The getBoxes function and values horizontal_count_, vertical_count_ and led_count_ are hardware specific.
 */
class Analyzer
{

  size_t horizontal_celldepth_ { 200 }; //!< The depth of the cells in horizontal direction.
  size_t vertical_celldepth_ { 200 }; //!< The depth of led cells in vertical direction.

public:
  static constexpr const size_t horizontal_count_ { 42 };  //!< Number of cells in horizontal direction.
  static constexpr const size_t vertical_count_ { 73 };    //!< Number of cells in vertical direction.
  static constexpr const size_t led_count_ { 228 };        //!< The number of leds in total.

  /**
   * @brief Return the number of leds that are used. 
   */
  size_t ledCount() const;

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
  Box findBorders(const Image& image, size_t bisects_per_side=4) const;

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
  void sample(const Image& screen, const Box& bounds, const std::vector<BoxSamples>& boxed_samples, std::vector<RGB>& canvas);

  /**
   * @brief Colorize a screen based on the colors in the canvas. This creates boxes on the edge that are 50 pixels deep.
   * @param canvas The canvas to draw on the screen.
   * @param[in, out] Outer borders of the image will get the boxes drawn on them.
   */
  void boxColorizer(const std::vector<RGB>& canvas, Image& image);

private:
  /**
   * @brief Create the box that's associated to each led for an rectangle of an arbritrary dimension.
   * @param width The width of the region the boxes should span.
   * @param height The height of the region the boxes should span.
   * @param horizontal_depth The horizontal depth of the cells on the left and right border.
   * @param vertical_depth The vertical depth of the cells on the top and bottom border.
   */
  std::vector<Box> getBoxes(size_t width, size_t height, size_t horizontal_depth, size_t vertical_depth);

};
#endif