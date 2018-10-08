#include "screen_analyzer.h"


RGB ScreenAnalyzer::average(const std::vector<std::vector<std::uint32_t>>& screen, size_t xmin, size_t ymin, size_t xmax, size_t ymax)
{
  uint32_t R = 0;
  uint32_t G = 0;
  uint32_t B = 0;
  for (size_t y=ymin; y < ymax; y++)
  {
    for (size_t x=xmin; x < xmax; x++)
    {
      const uint32_t color = screen[y][x];
      R += (color >> 16) & 0xFF;
      G += (color >> 8) & 0xFF;
      B += color & 0xFF;
    }
  }
  uint32_t total_possible = (xmax - xmin) * (ymax - ymin) * 255;
  //  std::cout << "total_possible: " << total_possible << std::endl;
  //  std::cout << "R: " << R << std::endl;
  //  std::cout << "G: " << G << std::endl;
  //  std::cout << "B: " << B << std::endl;
  R = R * 255 / total_possible;
  G = G * 255 / total_possible;
  B = B * 255 / total_possible;
  return {static_cast<uint8_t>(R), static_cast<uint8_t>(G), static_cast<uint8_t>(B)};
}

std::vector<RGB> ScreenAnalyzer::contentToCanvas(const std::vector<std::vector<std::uint32_t>>& screen)
{
  const size_t led_count = 228;
  auto canvas = std::vector<RGB>{led_count, {0, 0, 0}};
// left side 0 - 41 (starts top)
// bottom side: 42 - 113 (starts left)
// right side: 114 - 155 (starts bottom)
// top side: 156 - 227 (starts right)
  const auto height = screen.size();
  const auto width = screen.front().size();

  const size_t vertical_step = height / 42;
  const size_t horizontal_step = width / 73;

  const size_t horizontal_celldepth = 300;   // into the screen.
  const size_t vertical_celldepth = 500;   // into the screen.

  // do left first.
  for (size_t led = 0; led < led_count; led++)
  {
    size_t xmin, ymin, xmax, ymax;
    if (led < 42)
    {
      const uint32_t pos = led - 0;
      // left side.
      xmin = 0;
      xmax = horizontal_celldepth;
      ymin = pos * vertical_step;
      ymax = (pos + 1) * vertical_step;
    }
    else if (led < 114)
    {
      // bottom
      const uint32_t pos = led - 42;
      xmin = pos * horizontal_step;
      xmax = (pos + 1) * horizontal_step;
      ymin = height - vertical_celldepth;
      ymax = height;
    }
    else if (led < 156)
    {
      // right side.
      const uint32_t pos = led - 114;
      xmin = width - horizontal_celldepth;
      xmax = width;
      ymin = height - (pos + 1) * vertical_step;
      ymax = height - (pos + 0) * vertical_step;
    }
    else if (led < led_count + 1)
    {
      // top side
      const uint32_t pos = led - 156;
      xmin = width - (pos + 1) * horizontal_step;
      xmax = width - (pos + 0) * horizontal_step;
      ymin = 0;
      ymax = vertical_celldepth;
    }
    else
    {
      continue;
    }
    auto color = average(screen, xmin, ymin, xmax, ymax);
    canvas[led] = color;
  }
  return canvas;
}
