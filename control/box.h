#ifndef BOX_H
#define BOX_H

/**
 * @brief A rectangle.
 */
struct Box
{
  size_t x_min { 0 };  //!< The left bound.
  size_t x_max { 0 };  //!< The right bound.
  size_t y_min { 0 };  //!< The top bound.
  size_t y_max { 0 };  //!< The bottom bound.

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

  bool operator <(const Box &b) const
  {
    using ComparisonType = std::tuple<size_t, size_t, size_t, size_t>;
    return ComparisonType(x_min, y_min, x_max, y_max) < ComparisonType(b.x_min, b.y_min, b.x_max, b.y_max);
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