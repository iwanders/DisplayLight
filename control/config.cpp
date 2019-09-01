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
#include "config.h"
#include <fstream>
#include <iostream>
#include <sstream>

Condition::operator std::string() const
{
  std::stringstream ss;
  ss << "   min_width: " << min_width << "\n" << "   max_width: " << max_width << "\n"
        "   min_height: " << min_height << "\n" << "   max_height: " << max_height << std::endl;
  return ss.str();
}

bool Condition::applies(std::size_t width, std::size_t height) const
{
  return (min_width <= width) && (width <= max_width) && (min_height <= height) && (height <= max_height);
}

RegionConfig::operator std::string() const
{
  std::stringstream ss;
  ss << "Config: " << name << std::endl;
  ss << "  x_offset: " << x_offset << std::endl;
  ss << "  y_offset: " << y_offset << std::endl;
  ss << "  width: " << width << std::endl;
  ss << "  height: " << height << std::endl;
  ss << "  Condition\n" << std::string(condition) << std::endl;
  return ss.str();
}

DisplayLightConfig::operator std::string() const
{
  std::stringstream ss;
  ss << "Frame rate: " << frame_rate << std::endl;
  for (const auto& region_config : configs)
  {
    ss << std::string(region_config);
  }
  return ss.str();
}

DisplayLightConfig DisplayLightConfig::parse(const std::string& content)
{
  std::cout << content << std::endl;
  std::stringstream ss(content);
  std::string line;

  DisplayLightConfig res;
  res.configs.resize(0);
  RegionConfig current;


  while(std::getline(ss, line, '\n'))
  {
    if (line.empty())
    {
      continue;
    }
    if (line[0] == '#')
    {
      continue;
    }

    std::string element_name;
    std::stringstream tl = std::stringstream(line);
    std::getline(tl, element_name, ' ');
    if (element_name == "Config:")
    {
      if (current.name != "default")
      {
        res.configs.push_back(current);
        current = RegionConfig{};
      }
      std::string name;
      tl >> name;
      current.name = name;
      continue;
    }

    if (element_name == "x_offset:")
    {
      tl >> current.x_offset;
      continue;
    }
    if (element_name == "y_offset:")
    {
      tl >> current.y_offset;
      continue;
    }
    if (element_name == "width:")
    {
      tl >> current.width;
      continue;
    }
    if (element_name == "height:")
    {
      tl >> current.height;
      continue;
    }
    if (element_name == "condition.min_width:")
    {
      tl >> current.condition.min_width;
      continue;
    }
    if (element_name == "condition.max_width:")
    {
      tl >> current.condition.max_width;
      continue;
    }
    if (element_name == "condition.min_height:")
    {
      tl >> current.condition.min_height;
      continue;
    }
    if (element_name == "condition.max_height:")
    {
      tl >> current.condition.max_height;
      continue;
    }
    if (element_name == "frame_rate:")
    {
      tl >> res.frame_rate;
      continue;
    }
    std::cerr << "Unexpected config line: \"" << line << "\"" << std::endl;
  }
  res.configs.push_back(current);

  res.configs.push_back(RegionConfig{});

  return res;
}

DisplayLightConfig DisplayLightConfig::load(const std::string& filename)
{
  std::ifstream ifs(filename, std::ios::binary | std::ios::ate);

  std::string result;
  result.reserve(ifs.tellg());

  ifs.seekg(0, std::ios::beg);
  result.assign((std::istreambuf_iterator<char>(ifs)),
                std::istreambuf_iterator<char>());
  ifs.close();
  return parse(result);
}

void testConfigThing()
{
  std::string test = "frame_rate: 13.37\n#comment\nConfig: double_monitor\ncondition.min_width: 2000\nx_offset: 1920\nConfig: single_screen\ncondition.max_width: 1920\n";
  auto res = DisplayLightConfig::parse(test);
  std::cout << std::string(res) << std::endl;
}

RegionConfig DisplayLightConfig::getApplicable(std::size_t width, std::size_t height) const
{
  for (const auto& region : configs)
  {
    std::cout << "Checking: " << region.name << std::endl;
    if (region.condition.applies(width, height))
    {
      return region;
    }
  }
  return RegionConfig{};
}

