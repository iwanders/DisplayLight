#include "screen_analyzer.h"
#include <iostream>
#include <limits>


size_t ScreenAnalyzer::ledCount() const
{
  return led_count_;
}

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

std::vector<LedBox> ScreenAnalyzer::getBoxes(size_t width, size_t height, size_t horizontal_depth, size_t vertical_depth)
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
std::vector<RGB> ScreenAnalyzer::contentToCanvas(const std::vector<std::vector<std::uint32_t>>& screen)
{
  auto canvas = std::vector<RGB>{led_count_, {0, 0, 0}};
  auto boxes = getBoxes(screen.front().size(), screen.size(), 300, 200);

  // do left first.
  for (const auto& box : boxes)
  {
    canvas[box.index] = average(screen, box.x_min, box.y_min, box.x_max, box.y_max);
  }
  return canvas;
}


std::vector<Sample> ScreenAnalyzer::sampleCanvas(const std::vector<std::vector<std::uint32_t>>& screen, const size_t max_level)
{
  const auto height = screen.size();
  const auto width = screen.front().size();


  std::vector<Sample> res;

  std::function<bool(size_t, size_t, size_t, size_t, size_t)> recurser;
  auto worker = [&screen, &max_level, &res, &recurser](size_t xmin, size_t xmax, size_t ymin, size_t ymax, size_t level)
  {
    // We're handed in a region.
    /*
       xmin, ymin          xmax, ymin
        *------------------*
        |.        .       .|
        |                  |
        |                  |
        |.        .       .|
        |                  |
        |                  |
        |.        .       .|
        *------------------*
      xmin, ymax          xmax, ymax
    */
    if (level >= max_level)
    {
      return false;
    }

    const size_t xmid = ((xmax - xmin) / 2) + xmin;
    const size_t ymid = ((ymax - ymin) / 2) + ymin;

    const auto top_left = screen[ymin][xmin] & 0x00FFFFFF;
    const auto top_right = screen[ymin][xmax] & 0x00FFFFFF;
    const auto bottom_left = screen[ymax][xmin] & 0x00FFFFFF;
    const auto bottom_right = screen[ymax][xmax] & 0x00FFFFFF;
    const auto mid_left = screen[ymid][xmin] & 0x00FFFFFF;
    const auto mid_right = screen[ymid][xmax] & 0x00FFFFFF;
    const auto top_center = screen[ymin][xmid] & 0x00FFFFFF;
    const auto bottom_center = screen[ymax][xmid] & 0x00FFFFFF;
    const auto mid_center = screen[ymid][xmid] & 0x00FFFFFF;

    if (top_left || top_center || mid_left || mid_center)
    {
      recurser(xmin, xmid, ymin, ymid, level + 1);  // top left
    }
    if (top_center || top_right || mid_center || mid_right)
    {
      recurser(xmid, xmax, ymin, ymid, level + 1);  // top right
    }
    if (mid_left || mid_center || bottom_center || bottom_left)
    {
      recurser(xmin, xmid, ymid, ymax, level + 1);  // bottom left
    }
    if (mid_center || mid_right || bottom_right || bottom_center)
    {
      recurser(xmid, xmax, ymid, ymax, level + 1);  // bottom right
    }

    //  res.emplace_back(mid_left, xmin, ymid);
    //  res.emplace_back(mid_right, xmax, ymid);
    //  res.emplace_back(top_center, xmid, ymin);
    //  res.emplace_back(bottom_center, xmid, ymax);
    //  if (mid_left && top_center && bottom_center && mid_right)
    if (((mid_left && mid_right) || (bottom_center && top_center)) && mid_center)
    {
      res.emplace_back(mid_center, xmid, ymid);
    }

    return false;
  };
  recurser = worker;
  recurser(0, width-1, 0, height-1, 0);


  return res;
}

std::vector<LedBox> ScreenAnalyzer::getBoxesForSamples(const std::vector<Sample>& samples, size_t& x_min, size_t& y_min)
{
  // Run through analyzed to determine min and max of everything.
  x_min = std::numeric_limits<size_t>::max();
  size_t x_max = 0;
  y_min = std::numeric_limits<size_t>::max();
  size_t y_max = 0;
  for (const auto& sample : samples)
  {
    x_min = std::min(x_min, sample.x);
    x_max = std::max(x_max, sample.x);
    y_min = std::min(y_min, sample.y);
    y_max = std::max(y_max, sample.y);
  }

  // create boxes.
  auto boxes = getBoxes(x_max - x_min, y_max - y_min, horizontal_celldepth_, vertical_celldepth_);
  return boxes;
}

std::vector<std::vector<size_t>> ScreenAnalyzer::boxPacker(const std::vector<std::vector<std::uint32_t>>& screen, const std::vector<Sample>& samples)
{
  size_t x_min;
  size_t y_min;
  auto boxes = getBoxesForSamples(samples, x_min, y_min);
  std::vector<std::vector<size_t>> box_indices;
  box_indices.resize(boxes.size());

  // now, this is a bit silly... this can be done much better.
  for (size_t box_i = 0; box_i < boxes.size(); box_i++)
  {
    for (size_t sample_i = 0; sample_i < samples.size(); sample_i++)
    {
      const auto& sample = samples[sample_i];
      if (boxes[box_i].inside(sample.x - x_min, sample.y - y_min))
      {
        box_indices[box_i].push_back(sample_i);
      }
    }
  }

  return box_indices;
}

bool ScreenAnalyzer::sampledBoxer(const std::vector<Sample>& samples, const std::vector<std::vector<size_t>>& box_indices, std::vector<RGB>& canvas)
{
  for (size_t box_i = 0; box_i < box_indices.size(); box_i++)
  {
    const auto& cell_indices = box_indices[box_i];
    
    uint32_t R = 0;
    uint32_t G = 0;
    uint32_t B = 0;
    uint32_t total = 0;


    for (const auto& index : cell_indices)
    {
      if (index > samples.size())
      {
        // Number of samples doesn't match, this means that we got incorrect input data.
        return false;
      }
      const auto& sample = samples[index];
      //  const uint32_t color = screen[sample.y][sample.x];
      R += sample.color.R;
      G += sample.color.G;
      B += sample.color.B;
      total += 255;
    }

    if (total == 0)
    {
      std::cout << "box_i " << box_i << std::endl;
      continue;
    }
    R = R * 255 / total;
    G = G * 255 / total;
    B = B * 255 / total;
    canvas[box_i].R = R;
    canvas[box_i].G = G;
    canvas[box_i].B = B;
  }
  return true;
}

void ScreenAnalyzer::boxColorizer(const std::vector<RGB>& canvas, std::vector<std::vector<std::uint32_t>>& screen)
{
  auto samples = sampleCanvas(screen, 8);
  size_t x_min;
  size_t y_min;
  auto boxes = getBoxesForSamples(samples, x_min, y_min);
  // now that we have the boxes and the canvas, we can color each individual box.
  for (size_t box_i = 0; box_i < boxes.size(); box_i++)
  {
    const auto& box = boxes[box_i];
    const auto& color = canvas[box_i];
    // now, we run through the box.
    for (size_t y=box.y_min; y < box.y_max; y++)
    {
      for (size_t x=box.x_min; x < box.x_max; x++)
      {
        screen[y + y_min][x + x_min] = color.toUint32();
      }
    }
  }
}