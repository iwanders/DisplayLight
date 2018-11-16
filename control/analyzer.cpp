#include "analyzer.h"
#include <iostream>
#include <limits>
#include <algorithm>

LedBox::LedBox(size_t led_index, size_t xmin, size_t xmax, size_t ymin, size_t ymax): index{led_index}, x_min(xmin)
  , x_max(xmax), y_min(ymin), y_max(ymax)
{
}

LedBox::operator std::string() const
{
  std::stringstream ss;
  ss << "<" << x_min << ", " << y_min << " - " << x_max << ", " << y_max << ">";
  return ss.str();
}

size_t Analyzer::ledCount() const
{
  return led_count_;
}

void Analyzer::setCellDepth(size_t horizontal, size_t vertical)
{
  horizontal_celldepth_ = horizontal;
  vertical_celldepth_ = vertical;
}

void Analyzer::findBorders(const Image& screen, size_t& x_min, size_t& y_min, size_t& x_max, size_t& y_max, size_t bisects_per_side)
{
  // Create 4 vectors to hold the results of the bisection procedure.
  std::vector<size_t> x_min_v(bisects_per_side, 0);
  std::vector<size_t> x_max_v(bisects_per_side, screen.getWidth() - 1);
  std::vector<size_t> y_min_v(bisects_per_side, 0);
  std::vector<size_t> y_max_v(bisects_per_side, screen.getHeight() - 1);

  // lambda to perform the bisection procedure.
  auto bisect = [](auto f, auto& min, auto& max)
  {
    auto upper = f(max);
    auto lower = f(min);
    while ((max - min > 2) && (upper != lower))
    {
      upper = f(max);
      lower = f(min);
      const auto midpoint = (max + min) / 2;
      auto center = f(midpoint);
      if (center != lower)
      {
        max = midpoint;
      }
      else
      {
        min = midpoint;
      }
    }
  };

  // Perform the bisections.
  for (size_t i = 0 ; i < bisects_per_side; i++)
  {
    size_t tmp = 0;
    size_t mid_y = (screen.getHeight() - 1) / (bisects_per_side + 1) * (i + 1);
    size_t mid_x = (screen.getWidth() - 1) / (bisects_per_side + 1) * (i + 1);
      
    // Perform left bound
    tmp = mid_x;
    bisect([&](auto v) {
        return screen.pixel(v, mid_y) != 0;
      }, x_min_v[i], tmp);

    // Perform right bound.
    tmp = mid_x;
    bisect([&](auto v) {
        return screen.pixel(v, mid_y) != 0;
      }, tmp, x_max_v[i]);

    // Perform lower bound.
    tmp = mid_y;
    bisect([&](size_t v) {
        return screen.pixel(mid_x, v) != 0;
      }, y_min_v[i], tmp);

    // Perform upper bound.
    tmp = mid_y;
    bisect([&](auto v) {
        return screen.pixel(mid_x, v) != 0;
      }, tmp, y_max_v[i]);
  }

  // Get the max or min of each border's bisection results.
  x_min = *std::min_element(x_min_v.begin(), x_min_v.end());
  y_min = *std::min_element(y_min_v.begin(), y_min_v.end());
  x_max = *std::max_element(x_max_v.begin(), x_max_v.end());
  y_max = *std::max_element(y_max_v.begin(), y_max_v.end());
}

void Analyzer::sampleBoxedSamples(const Image& screen, const size_t x_min, const size_t y_min, const std::vector<BoxedSamples>& boxed_samples, std::vector<RGB>& canvas)
{
  for (size_t box_i = 0; box_i < boxed_samples.size(); box_i++)
  {
    auto& box = boxed_samples[box_i];
    auto& canvas_pixel = canvas[box_i];

    // Run over the sample points and create average color.
    uint32_t R = 0;
    uint32_t G = 0;
    uint32_t B = 0;
    uint32_t total = 0;
    for (const auto& p : box.points)
    {
      const uint32_t color = screen.pixel(p.first + x_min,p.second + y_min);
      R += (color >> 16) & 0xFF;
      G += (color >> 8) & 0xFF;
      B += color & 0xFF;
      total += 255;
    }

    if (total == 0)
    {
      // This can only happen if there are no samples in the box, which should never happen.
      throw std::runtime_error("No samples in this box: " + std::string(box.box));
    }
    R = R * 255 / total;
    G = G * 255 / total;
    B = B * 255 / total;

    // Assign the calculated color to the canvas.
    canvas_pixel.R = R;
    canvas_pixel.G = G;
    canvas_pixel.B = B;
  }  
}

std::vector<BoxedSamples> Analyzer::makeBoxedSamplePoints(const size_t dist_between_samples, const size_t x_min,
  const size_t y_min, const size_t x_max, const size_t y_max)
{
  // Get the boxes associated to these bounds.
  auto boxes = getBoxes(x_max - x_min, y_max - y_min, horizontal_celldepth_, vertical_celldepth_);
  std::vector<BoxedSamples> res{boxes.size()};

  for (size_t i = 0; i < boxes.size(); i++)
  {
    const auto& box = res[i].box;
    res[i].box = boxes[i];
    
    // now add samples.
    for (size_t y = box.y_min; y < box.y_max ; y += dist_between_samples)
    {
      for (size_t x = box.x_min; x < box.x_max ; x += dist_between_samples)
      {
        res[i].points.emplace_back(x, y);
      }
    }
  }
  return res;
}


void Analyzer::boxColorizer(const std::vector<RGB>& canvas, Image& screen)
{
  auto boxes = getBoxes(screen.getWidth(), screen.getHeight(), 50, 50);

  // now that we have the boxes and the canvas, we can color each individual box.
  for (size_t box_i = 0; box_i < boxes.size(); box_i++)
  {
    const auto& box = boxes[box_i];
    const auto& color = canvas[box_i];
    // now, we run through the box and fill it with the color.
    for (size_t y=box.y_min; y < box.y_max; y++)
    {
      for (size_t x=box.x_min; x < box.x_max; x++)
      {
        screen.setPixel(x, y, color.toUint32());
      }
    }
  }
}

std::vector<LedBox> Analyzer::getBoxes(size_t width, size_t height, size_t horizontal_depth, size_t vertical_depth)
{
  std::vector<LedBox> res;
  res.reserve(led_count_);

  // left side 0 - 41 (starts top)
  // bottom side: 42 - 113 (starts left)
  // right side: 114 - 155 (starts bottom)
  // top side: 156 - 227 (starts right)
  const size_t vertical_step = height / horizontal_count_;
  const size_t horizontal_step = width / vertical_count_;

  // do left first.
  for (size_t led = 0; led < led_count_; led++)
  {
    size_t xmin, ymin, xmax, ymax;
    if (led < 42)
    {
      const uint32_t pos = led - 0;
      // left side.
      xmin = 0;
      xmax = horizontal_depth;
      ymin = pos * vertical_step;
      ymax = (pos + 1) * vertical_step;
      res.emplace_back(led, xmin, xmax, ymin, ymax);
    }
    else if (led < 114)
    {
      // bottom
      const uint32_t pos = led - 42;
      xmin = pos * horizontal_step;
      xmax = (pos + 1) * horizontal_step;
      ymin = height - vertical_depth;
      ymax = height;
      res.emplace_back(led, xmin, xmax, ymin, ymax);
    }
    else if (led < 156)
    {
      // right side.
      const uint32_t pos = led - 114;
      xmin = width - horizontal_depth;
      xmax = width;
      ymin = height - (pos + 1) * vertical_step;
      ymax = height - (pos + 0) * vertical_step;
      res.emplace_back(led, xmin, xmax, ymin, ymax);
    }
    else if (led < led_count_ + 1)
    {
      // top side
      const uint32_t pos = led - 156;
      xmin = width - (pos + 1) * horizontal_step;
      xmax = width - (pos + 0) * horizontal_step;
      ymin = 0;
      ymax = vertical_depth;
      res.emplace_back(led, xmin, xmax, ymin, ymax);
    }
  }
  return res;
}
