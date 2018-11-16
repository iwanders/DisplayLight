#include "analyzer.h"
#include <iostream>
#include <limits>
#include <algorithm>

std::vector<RGB> Analyzer::makeCanvas()
{
  return Lights::makeCanvas();
}

void Analyzer::setCellDepth(size_t horizontal, size_t vertical)
{
  horizontal_celldepth_ = horizontal;
  vertical_celldepth_ = vertical;
}

Box Analyzer::findBorders(const Image& image, size_t bisects_per_side) const
{
  // Create 4 vectors to hold the results of the bisection procedure.
  std::vector<size_t> x_min_v(bisects_per_side, 0);
  std::vector<size_t> x_max_v(bisects_per_side, image.getWidth() - 1);
  std::vector<size_t> y_min_v(bisects_per_side, 0);
  std::vector<size_t> y_max_v(bisects_per_side, image.getHeight() - 1);

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
    size_t mid_y = (image.getHeight() - 1) / (bisects_per_side + 1) * (i + 1);
    size_t mid_x = (image.getWidth() - 1) / (bisects_per_side + 1) * (i + 1);
      
    // Perform left bound
    tmp = mid_x;
    bisect([&](auto v) {
        return image.pixel(v, mid_y) != 0;
      }, x_min_v[i], tmp);

    // Perform right bound.
    tmp = mid_x;
    bisect([&](auto v) {
        return image.pixel(v, mid_y) != 0;
      }, tmp, x_max_v[i]);

    // Perform lower bound.
    tmp = mid_y;
    bisect([&](size_t v) {
        return image.pixel(mid_x, v) != 0;
      }, y_min_v[i], tmp);

    // Perform upper bound.
    tmp = mid_y;
    bisect([&](auto v) {
        return image.pixel(mid_x, v) != 0;
      }, tmp, y_max_v[i]);
  }

  // Get the max or min of each border's bisection results.
  Box bounds;
  bounds.x_min = *std::min_element(x_min_v.begin(), x_min_v.end());
  bounds.y_min = *std::min_element(y_min_v.begin(), y_min_v.end());
  bounds.x_max = *std::max_element(x_max_v.begin(), x_max_v.end());
  bounds.y_max = *std::max_element(y_max_v.begin(), y_max_v.end());
  return bounds;
}

void Analyzer::sample(const Image& screen, const Box& bounds, const std::vector<BoxSamples>& boxed_samples, std::vector<RGB>& canvas)
{
  for (size_t box_i = 0; box_i < boxed_samples.size(); box_i++)
  {
    const auto& box = boxed_samples[box_i];
    auto& canvas_pixel = canvas[box_i];

    // Run over the sample points and create average color.
    uint32_t R = 0;
    uint32_t G = 0;
    uint32_t B = 0;
    uint32_t total = 0;
    for (const auto& p : box.points)
    {
      const uint32_t color = screen.pixel(p.first + bounds.x_min, p.second + bounds.y_min);
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

    // Assign the calculated average color to the canvas.
    canvas_pixel.R = R * 255 / total;
    canvas_pixel.G = G * 255 / total;
    canvas_pixel.B = B * 255 / total;
  }  
}

std::vector<BoxSamples> Analyzer::makeBoxSamples(const size_t dist_between_samples, const Box& bounds)
{
  // Get the boxes associated to these bounds.
  auto boxes = Lights::getBoxes(bounds.width(), bounds.height(), horizontal_celldepth_, vertical_celldepth_);
  std::vector<BoxSamples> res{boxes.size()};

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


void Analyzer::boxColorizer(const std::vector<RGB>& canvas, Image& image)
{
  auto boxes = Lights::getBoxes(image.getWidth(), image.getHeight(), 50, 50);

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
        image.setPixel(x, y, color.toUint32());
      }
    }
  }
}
