#ifndef SCREEN_ANALYZER_H
#define SCREEN_ANALYZER_H

#include <vector>
#include <functional>
#include <cstdint>
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

class ScreenAnalyzer
{
  const size_t horizontal_count_ { 42 };  //!< Number of cells in horizontal direction.
  const size_t vertical_count_ { 73 };    //!< Number of cells in vertical direction.
  const size_t led_count_ { 228 };        //!< The number of leds in total.
public:

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

  std::vector<Sample> sampleCanvas(const std::vector<std::vector<std::uint32_t>>& screen);

};
#endif