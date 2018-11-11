#ifndef SCREEN_ANALYZER_H
#define SCREEN_ANALYZER_H

#include <vector>
#include <functional>
#include <cstdint>
#include <sstream>
#include "../firmware/messages.h"

struct Sample
{
  Sample() = default;
  Sample(size_t colori, size_t xi, size_t yi) : x(xi), y(yi), color({(colori >> 16) & 0xFF, (colori >> 8) & 0xFF, colori & 0xFF}){};
  size_t x { 0 };
  size_t y { 0 };
  size_t level { 0 };
  RGB color { 0, 0, 0 };
};

struct LedBox
{
  size_t index { 0 };
  size_t x_min { 0 };
  size_t x_max { 0 };
  size_t y_min { 0 };
  size_t y_max { 0 };
  LedBox(size_t led_index, size_t xmin, size_t xmax, size_t ymin, size_t ymax): index{led_index}, x_min(xmin), x_max(xmax), y_min(ymin), y_max(ymax){};
  bool inside(size_t x, size_t y) const
  {
    return (x_min <= x) && (x <= x_max) && (y_min <= y) && (y <= y_max);
  };
   operator std::string() const
  {
    std::stringstream ss;
    ss << "<" << x_min << ", " << y_min << " - " << x_max << ", " << y_max << ">";
    return ss.str();
  }
};

class ScreenAnalyzer
{
  const size_t horizontal_count_ { 42 };  //!< Number of cells in horizontal direction.
  const size_t vertical_count_ { 73 };    //!< Number of cells in vertical direction.
  const size_t led_count_ { 228 };        //!< The number of leds in total.

  const size_t horizontal_celldepth_ { 100 }; //!< The depth of the cells in horizontal direction.
  const size_t vertical_celldepth_ { 100 }; //!< The depth of led cells in vertical direction.

public:
  size_t ledCount() const;

  std::vector<LedBox> getBoxes(size_t width, size_t height, size_t horizontal_depth, size_t vertical_depth);

  std::vector<LedBox> getBoxesForSamples(const std::vector<Sample>& samples, size_t& x_min, size_t& y_min);

  /**
   * @brief Averages a region of a the screen.
   * @param screen The content to average from.
   * @param xmin, ymin, xmax, ymax The bounds of the region to verage.
   */
  RGB average(const std::vector<std::vector<std::uint32_t>>& screen, size_t xmin, size_t ymin, size_t xmax, size_t ymax);

  /**
   * @brief Converts a screen to a vector of cell colors using averaging.
   */
  std::vector<RGB> contentToCanvas(const std::vector<std::vector<std::uint32_t>>& screen);

  /**
   * @brief Sample the canvas, recursing down using a quadtree-like approach. This should find the contents save for
   *        pure black borders.
   */
  std::vector<Sample> sampleCanvas(const std::vector<std::vector<std::uint32_t>>& screen, const size_t max_level);

  /**
   * @brief Determine the mapping box each sample is associated with.
   */
  std::vector<std::vector<size_t>> boxPacker(const std::vector<std::vector<std::uint32_t>>& screen, const std::vector<Sample>& samples);

  /**
   * 
   */
  bool sampledBoxer(const std::vector<Sample>& samples, const std::vector<std::vector<size_t>>& box_indices, std::vector<RGB>& canvas);

  void boxColorizer(const std::vector<RGB>& canvas, std::vector<std::vector<std::uint32_t>>& screen);
};
#endif