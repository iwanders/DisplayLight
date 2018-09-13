#include <boost/asio.hpp>
#include <iostream>
#include "../firmware/messages.h"
#include "pixelsniff.h"


#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

// Inclusive bounds:
// left side 0 - 41 (starts top)
// bottom side: 41 - 113 (starts left)
// right side: 114 - 155 (starts bottom)
// top side: 156 - 227 (starts right)

const size_t led_count = 228;

std::vector<Message> chunker(const std::array<RGB, led_count>& buffer)
{
  std::vector<Message> res;
  Message x;
  x.type = COLOR;
  x.color.offset = 0;
  size_t index = 0;
  for (uint8_t i = 0; i < buffer.size(); i++)
  {
    x.color.offset = (index / 19) * 19;
    x.color.color[index % 19] = buffer[i];

    index++;
    if (index % 19 == 0)
    {
      res.push_back(x);
    }
  }

  res.back().color.settings = COLOR_SETTINGS_SHOW_AFTER;
  return res;
}

std::array<RGB, led_count> empty(const RGB v = { 0, 0, 0 })
{
  std::array<RGB, led_count> res;
  res.fill(v);
  return res;
}


void write(boost::asio::serial_port& serial, const std::array<RGB, led_count>& canvas)
{
  auto msgs = chunker(canvas);
  for (const auto& msg : msgs)
  {
    boost::asio::write(serial, boost::asio::buffer(&msg, sizeof(msg)));
  }
}

void limiter(std::array<RGB, led_count>& canvas)
{
  for (auto& rgb : canvas)
  {
    rgb.R /= 8;
    rgb.G /= 8;
    rgb.B /= 8;
  }
}

RGB average(const std::vector<std::vector<uint32_t>>& screen, size_t xmin, size_t ymin, size_t xmax, size_t ymax)
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

std::array<RGB, led_count> contentToCanvas(const std::vector<std::vector<uint32_t>>& screen)
{
  auto canvas = empty();
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

std::array<RGB, led_count>  boundsCanvas()
{
  auto z = empty();
  z[41].R = 255;
  z[41].B = 255;

  z[42].G = 255;
  z[113].B = 255;

  z[114].R = 255;
  z[155].B = 255;

  z[156].B = 255;
  z[156].G = 255;

  z[227].G = 255;
  return z;
}

void printCanvas(const std::array<RGB, led_count>& canvas)
{
  for (const auto& color : canvas)
  {
    uint32_t R = color.R;
    uint32_t G = color.G;
    uint32_t B = color.B;
    std::cout << "(" << R << ", " << G << ", " << B << ") ";
  }
  std::cout << std::endl;
}

int main(int argc, char* argv[])
{
  PixelSniffer sniff;
  sniff.populate();
  sniff.selectWindow(0);  // 0 is the root window.
  sniff.grabContent();
  auto content = sniff.content();
  // some helper functions
  auto start = std::chrono::steady_clock::now();
  auto tic = [&start]() { start = std::chrono::steady_clock::now(); };
  auto toc = [&start](bool print = false) {
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    size_t count = std::chrono::duration<double, std::micro>(diff).count();
    if (print)
    {
      std::cout << count << " us" << std::endl;
    }
    return count;
  };

  try
  {
    boost::asio::io_service io;
    boost::asio::serial_port serial{ io, argv[1] };
    serial.set_option(boost::asio::serial_port_base::baud_rate(115200));

    while (0)
    {
      for (uint8_t i = 0; i < led_count; i++)
      {
        auto canvas = empty();
        canvas[i].R = 255;
        write(serial, canvas);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    }

    
  size_t cumulative = 0;
  size_t count = 0;
    while (1)
    {
      tic();
      bool res = sniff.grabContent();
      sniff.content(content);
      //  auto content = sniff.content();
      auto canvas = contentToCanvas(content);
      //  printCanvas(canvas);
      limiter(canvas);
      write(serial, canvas);
      cumulative += toc();
      count++;
      if (res)
      {
        std::cout << "Grab succesful" << std::endl;
        std::cout << "iters done:" << count << " avg: " << double(cumulative) / count << " usec" << std::endl;
        //  std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      
    }
  }
  catch (boost::system::system_error& e)
  {
    std::cout << "Error: " << e.what() << std::endl;
    return 1;
  }
}
