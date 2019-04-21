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
#ifndef BOX_H
#define BOX_H

/**
 * @brief A rectangle.
 */
struct Box
{
  size_t x_min{ 0 };  //!< The left bound.
  size_t x_max{ 0 };  //!< The right bound.
  size_t y_min{ 0 };  //!< The top bound.
  size_t y_max{ 0 };  //!< The bottom bound.

  Box() = default;
  Box(size_t xmin, size_t xmax, size_t ymin, size_t ymax) : x_min(xmin), x_max(xmax), y_min(ymin), y_max(ymax)
  {
  }

  operator std::string() const
  {
    std::stringstream ss;
    ss << "<" << x_min << ", " << y_min << " - " << x_max << ", " << y_max << ">";
    return ss.str();
  }

  bool operator<(const Box& b) const
  {
    using ComparisonType = std::tuple<size_t, size_t, size_t, size_t>;
    return ComparisonType(x_min, y_min, x_max, y_max) < ComparisonType(b.x_min, b.y_min, b.x_max, b.y_max);
  }

  bool operator==(const Box& b) const
  {
    using ComparisonType = std::tuple<size_t, size_t, size_t, size_t>;
    return ComparisonType(x_min, y_min, x_max, y_max) == ComparisonType(b.x_min, b.y_min, b.x_max, b.y_max);
  }

  size_t width() const
  {
    return x_max - x_min;
  }

  size_t height() const
  {
    return y_max - y_min;
  }
};

#endif