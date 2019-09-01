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
#include <chrono>
#include <iostream>
#include <vector>

#include "analyzer.h"
#include "lights.h"
#include "pixelsniff.h"
#include "platform.h"
#include "timing.h"
#include "config.h"

void printHelp(const std::string& progname)
{
  std::cout << "" << progname << " serial_port_path [config]" << std::endl;
}

int main(int argc, char* argv[])
{
  // testConfigThing();
  // return 0;

  PixelSniffer::Ptr sniff = getSniffer();
  sniff->connect();
  sniff->selectRootWindow();

  Lights lights;
  Analyzer analyzer;
  DisplayLightConfig config;

  std::string path = "COM5";

  // Handle help printing
  if (argc < 2)
  {
    printHelp(argv[0]);
    return 1;
  }
  if ((argc >= 2) && (std::string(argv[1]) == "--help"))
  {
    printHelp(argv[0]);
    return 1;
  }

  // set the path
  if (argc >= 2)
  {
    path = argv[1];
  }
  // load the config
  if (argc >= 3)
  {
    config = DisplayLightConfig::load(argv[2]);
  }

  Limiter limiter{ config.frame_rate };

  // Try to connect to the provided serial port.
  if (!lights.connect(path))
  {
    std::cout << "Failed to connect to " << path << std::endl;
    return 1;
  }

  const size_t distance_between_sample_pixels{ 15 };

  // Create the canvas
  std::vector<RGB> canvas{ lights.ledCount(), { 0, 0, 0 } };

  Box sample_bounds;
  std::vector<BoxSamples> sample_points;
  // sniff.prepareCapture(x, y, w, h);
  PixelSniffer::Resolution current_res;

  while (1)
  {
    // Rate limit the loop.
    limiter.sleep();

    // Grab the contents of the screen.
    bool success = sniff->grabContent();
    if (!success)
    {
      // This happens on timeout in windows... just delay 1 millisecond and try again.
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    const auto image = sniff->getScreen();

    const Box bounds = analyzer.findBorders(*image);
    if (!(bounds == sample_bounds))
    {
      sample_points = analyzer.makeBoxSamples(distance_between_sample_pixels, bounds);
      sample_bounds = bounds;
    }
    analyzer.sample(*image, bounds, sample_points, canvas);
    lights.write(canvas);


    if (current_res != sniff->getFullResolution())
    {
      // Check which one applies.
      current_res = sniff->getFullResolution();
      std::cout << "New res: " << current_res.first << " x " << current_res.second << std::endl;
      auto regionconfig = config.getApplicable(current_res.first, current_res.second);
      std::cout << "Detected dimension change, applicable config: " << regionconfig.name << std::endl;
      sniff->prepareCapture(regionconfig.x_offset, regionconfig.y_offset, regionconfig.width, regionconfig.height);
    }
  }
}
