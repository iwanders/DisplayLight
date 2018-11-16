#include <boost/asio.hpp>
#include <iostream>
#include "../firmware/messages.h"
#include "pixelsniff.h"
#include "analyzer.h"


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
    rgb.R /= 2;
    rgb.G /= 2;
    rgb.B /= 2;
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

int main(int /* argc */, char* argv[])
{
  PixelSniffer sniff;
  Analyzer analyzer;
  sniff.connect();
  sniff.selectRootWindow();
  sniff.grabContent();
  //  auto content = sniff.content();
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
    serial.set_option(boost::asio::serial_port_base::baud_rate(1152000));

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
    std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point b = std::chrono::system_clock::now();


    using Bounds = std::tuple<size_t, size_t, size_t, size_t>;
    std::map<Bounds, std::vector<BoxedSamples>> cache;
    std::vector<RGB> canvas{analyzer.ledCount(), {0, 0, 0}};
    while (1)
    {


      // Maintain designated frequency of 5 Hz (200 ms per frame)
      a = std::chrono::system_clock::now();
      std::chrono::duration<double, std::milli> work_time = a - b;
      //  const double frame_period = 5000.0;  //
      //  const double frame_period = 100.0;  // 10 Hz

      const size_t frame_period = 16.0;  // 60 Hz
      //  const size_t frame_period = 1000 / 30;  // 30 Hz
      if (work_time.count() < frame_period)
      {
          std::chrono::duration<double, std::milli> delta_ms(frame_period - work_time.count());
          auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
          //  std::cout << "Sleeping for: " << delta_ms_duration.count() << std::endl;
          std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
      }

      b = std::chrono::system_clock::now();
      std::chrono::duration<double, std::milli> sleep_time = b - a;

      //  tic();
      bool res = sniff.grabContent();
      //  sniff.content(content);
      const auto content = sniff.getScreen();
      //  std::cout << "Grab: " << res << "  ";
      //  toc(true);

      tic();
      //=================
      // perform bisection.
      Bounds current;
      analyzer.findBorders(content, std::get<0>(current), std::get<1>(current), std::get<2>(current), std::get<3>(current));
      // check if present in the cache.
      //  if (cache.find(current) == cache.end())
      {
     
        // not in the cache, quickly, make the box points.
        cache[current] = analyzer.makeBoxedSamplePoints(15, std::get<0>(current), std::get<1>(current), std::get<2>(current), std::get<3>(current));   
        std::cout << "Making boxed points." << ", " <<  std::get<0>(current)<< ", " <<  std::get<1>(current)<< ", " <<  std::get<2>(current)<< ", " <<  std::get<3>(current) << std::endl;
        std::cout << "   Samples per cell: " << cache[current].front().points.size() << std::endl;
      }

      // perform the analyses.
      analyzer.sampleBoxedSamples(content, std::get<0>(current), std::get<1>(current), cache[current], canvas);
      cumulative += toc();
      
      //===============
      limiter(canvas);
      write(serial, canvas);
      count++;
      if (res)
      {
        std::cout << "iters done:" << count << " avg: " << double(cumulative) / count << " usec" << std::endl;
        //  std::this_thread::sleep_for(std::chrono::milliseconds(10));

        //  analyzer.boxColorizer(canvas, content);
        //  std::ofstream outcontent("/tmp/lastframe.ppm");
        //  outcontent << sniff.imageToPPM(content);
        //  outcontent.close();
      }
    }
  }
  catch (boost::system::system_error& e)
  {
    std::cout << "Error: " << e.what() << std::endl;
    return 1;
  }
}
