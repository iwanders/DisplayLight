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
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <cstdint>
#include <vector>
#include <limits>

struct Condition
{
  std::size_t min_width{ 0 };
  std::size_t max_width{ std::numeric_limits<std::size_t>::max() };
  std::size_t min_height{ 0 };
  std::size_t max_height{ std::numeric_limits<std::size_t>::max() };

  bool applies(std::size_t width, std::size_t height) const;
  operator std::string() const;
};

struct RegionConfig
{
  std::string name{"default"};
  Condition condition;


  std::size_t x_offset{ 0 };
  std::size_t y_offset{ 0 };
  std::size_t width{ std::numeric_limits<std::size_t>::max() };
  std::size_t height{ std::numeric_limits<std::size_t>::max() };

  operator std::string() const;
};


struct DisplayLightConfig
{

  std::vector<RegionConfig> configs {RegionConfig{}};

  double frame_rate{ 60 };

  RegionConfig getApplicable(std::size_t width, std::size_t height) const;

  operator std::string() const;

  static DisplayLightConfig parse(const std::string& content);
  static DisplayLightConfig load(const std::string& filename);
};

void testConfigThing();

#endif
