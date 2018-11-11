#ifndef SCREEN_ANALYZER_H
#define SCREEN_ANALYZER_H

#include <vector>
#include <functional>
#include <cstdint>
#include <sstream>
#include "../firmware/messages.h"
#include "backed_screen.h"

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
  LedBox() = default;
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

struct BoxedSamples
{
  LedBox box;
  std::vector<std::pair<size_t, size_t>> points;
};

class ScreenAnalyzer
{
  const size_t horizontal_count_ { 42 };  //!< Number of cells in horizontal direction.
  const size_t vertical_count_ { 73 };    //!< Number of cells in vertical direction.
  const size_t led_count_ { 228 };        //!< The number of leds in total.

  const size_t horizontal_celldepth_ { 100 }; //!< The depth of the cells in horizontal direction.
  const size_t vertical_celldepth_ { 100 }; //!< The depth of led cells in vertical direction.

public:
  using Screen = std::vector<std::vector<uint32_t>>;
  using BoxIndices = std::vector<std::vector<size_t>>;
  using Samples = std::vector<Sample>;

  size_t ledCount() const;

  std::vector<LedBox> getBoxes(size_t width, size_t height, size_t horizontal_depth, size_t vertical_depth);

  /**
   * @brief Make boxed samplepoints.
   */
  std::vector<BoxedSamples> makeBoxedSamplePoints(const size_t dist_between_samples, const size_t x_min, const size_t y_min, const size_t x_max, const size_t y_max);

  /**
   * @brief Perform 4 bisection procedures to find the left, bottom, top and right borders.
   */
  void findBorders(const BackedScreen& screen, size_t& x_min, size_t& y_min, size_t& x_max, size_t& y_max);


  void sampleBoxedSamples(const BackedScreen& screen, const size_t x_min, const size_t y_min, const std::vector<BoxedSamples>& boxed_samples, std::vector<RGB>& canvas);

  /**
   * @brief Colorize the screen based on a canvas.
   */
  void boxColorizer(const std::vector<RGB>& canvas, BackedScreen& screen);
};
#endif