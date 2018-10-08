#include <boost/asio.hpp>
#include <iostream>
#include "../firmware/messages.h"
#include "pixelsniff.h"
#include "screen_analyzer.h"


#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <thread>
#include <vector>

// Inclusive bounds:
// left side 0 - 41 (starts top)
// bottom side: 41 - 113 (starts left)
// right side: 114 - 155 (starts bottom)
// top side: 156 - 227 (starts right)

const size_t led_count = 228;

std::vector<Message> chunker(const std::vector<RGB>& buffer)
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

std::vector<RGB> empty(const RGB v = { 0, 0, 0 })
{
  std::vector<RGB> res{led_count, v};
  return res;
}


void write(boost::asio::serial_port& serial, const std::vector<RGB>& canvas)
{
  auto msgs = chunker(canvas);
  for (const auto& msg : msgs)
  {
    boost::asio::write(serial, boost::asio::buffer(&msg, sizeof(msg)));
  }
}

void limiter(std::vector<RGB>& canvas)
{
  for (auto& rgb : canvas)
  {
    rgb.R /= 8;
    rgb.G /= 8;
    rgb.B /= 8;
  }
}

std::vector<RGB> boundsCanvas()
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

void printCanvas(const std::vector<RGB>& canvas)
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
  ScreenAnalyzer analyzer;
  sniff.connect();
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
      auto canvas = analyzer.contentToCanvas(content);
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
