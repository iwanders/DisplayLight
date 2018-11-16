
#include <iostream>
#include "pixelsniff.h"
#include "analyzer.h"
#include "lights.h"


#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <thread>
#include <vector>

int main(int /* argc */, char* argv[])
{
  PixelSniffer sniff;
  sniff.connect();
  sniff.selectRootWindow();

  Analyzer analyzer;

  Lights lights;

  if (!lights.connect(argv[1]))
  {
    std::cout << "Failed to connect to " << argv[1] << std::endl;
    return 1;
  }

  std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
  std::chrono::system_clock::time_point b = std::chrono::system_clock::now();


  std::map<Box, std::vector<BoxSamples>> cache;
  std::vector<RGB> canvas{lights.ledCount(), {0, 0, 0}};
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

    bool success = sniff.grabContent();
    if (!success)
    {
      std::cerr << "Could not grab desired contents. Quitting." << std::endl;
      return 1;
    }
    const auto content = sniff.getScreen();
    Box current = analyzer.findBorders(content);
    auto samplepoints = analyzer.makeBoxSamples(15, current);
    analyzer.sample(content, current, samplepoints, canvas);

    lights.write(canvas);
  }

}
