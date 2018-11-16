#include "lights.h"
#include <vector>
#include <iostream>


bool Lights::connect(const std::string& serial_path, size_t baudrate)
{
  try
  {
    serial_ = std::make_unique<boost::asio::serial_port>( io_, serial_path);
    serial_->set_option(boost::asio::serial_port_base::baud_rate(baudrate));
  }
  catch (boost::system::system_error& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return false;
  }
  return true;
}

std::vector<Message> Lights::chunker(const std::vector<RGB>& canvas) const
{
  std::vector<Message> res;

  // Setup message to set colors.
  Message msg;
  msg.type = COLOR;
  msg.color.offset = 0;

  const size_t leds_per_message = msg.color.leds_per_message;
  for (size_t index = 0; index < canvas.size(); index++)
  {
    // Set the offset of this message, flooring the index
    msg.color.offset = (index / leds_per_message) * leds_per_message;

    // Set the color with the desired color from the canvas
    msg.color.color[index % leds_per_message] = canvas[index];

    // Limit the color
    limiter(msg.color.color[index % leds_per_message]);

    if ((index + 1) % leds_per_message == 0) // If message is complete, push it into the vector.
    {
      res.push_back(msg);
    }
  }

  // Tell the hardware to actually set the message after.
  res.back().color.settings = msg.color.settings_show_after;
  return res;
}

void Lights::fill(const RGB v) const
{
  std::vector<RGB> res{led_count_, v};
  write(res);
}


void Lights::write(const std::vector<RGB>& canvas) const
{
  auto msgs = chunker(canvas);
  for (const auto& msg : msgs)
  {
    boost::asio::write(*serial_, boost::asio::buffer(&msg, sizeof(msg)));
  }
}

void Lights::limiter(RGB& rgb) const
{
  rgb.R *= limit_factor_;
  rgb.G *= limit_factor_;
  rgb.B *= limit_factor_;
}

void Lights::setLimitFactor(double factor)
{
  limit_factor_ = factor;
}

void Lights::writeBoundsCanvas() const
{
  auto z = makeCanvas();
  z[41].R = 255;
  z[41].B = 255;

  z[42].G = 255;
  z[113].B = 255;

  z[114].R = 255;
  z[155].B = 255;

  z[156].B = 255;
  z[156].G = 255;

  z[227].G = 255;
  write(z);
}

std::vector<Box> Lights::getBoxes(size_t width, size_t height, size_t horizontal_depth, size_t vertical_depth)
{
  std::vector<Box> res;
  res.reserve(led_count_);

  // Inclusive bounds:
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
      res.emplace_back(xmin, xmax, ymin, ymax);
    }
    else if (led < 114)
    {
      // bottom
      const uint32_t pos = led - 42;
      xmin = pos * horizontal_step;
      xmax = (pos + 1) * horizontal_step;
      ymin = height - vertical_depth;
      ymax = height;
      res.emplace_back(xmin, xmax, ymin, ymax);
    }
    else if (led < 156)
    {
      // right side.
      const uint32_t pos = led - 114;
      xmin = width - horizontal_depth;
      xmax = width;
      ymin = height - (pos + 1) * vertical_step;
      ymax = height - (pos + 0) * vertical_step;
      res.emplace_back(xmin, xmax, ymin, ymax);
    }
    else if (led < led_count_ + 1)
    {
      // top side
      const uint32_t pos = led - 156;
      xmin = width - (pos + 1) * horizontal_step;
      xmax = width - (pos + 0) * horizontal_step;
      ymin = 0;
      ymax = vertical_depth;
      res.emplace_back(xmin, xmax, ymin, ymax);
    }
  }
  return res;
}

std::vector<RGB> Lights::makeCanvas()
{
  std::vector<RGB> canvas{ledCount(), {0, 0, 0}};
  return canvas;
}
