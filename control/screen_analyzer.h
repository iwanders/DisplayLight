#ifndef SCREEN_ANALYZER_H
#define SCREEN_ANALYZER_H

#include <vector>
#include <cstdint>
#include "../firmware/messages.h"

class ScreenAnalyzer
{
public:
RGB average(const std::vector<std::vector<std::uint32_t>>& screen, size_t xmin, size_t ymin, size_t xmax, size_t ymax);
std::vector<RGB> contentToCanvas(const std::vector<std::vector<std::uint32_t>>& screen);
};
#endif