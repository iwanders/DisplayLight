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
#include <iostream>
#include <vector>

#include "analyzer.h"
#include "lights.h"
#include "pixelsniff.h"
#include "platform.h"
#include "timing.h"

void printHelp(const std::string& progname)
{

  std::cout << "" << progname << " serial_port_path [framerate_in_hz]" << std::endl;
}

int main(int argc, char* argv[])
{
  PixelSniffer::Ptr sniff = getSniffer();
  sniff->connect();
  sniff->selectRootWindow();

  Lights lights;
  Analyzer analyzer;

  std::string path = "COM5";
  double framerate{ 60 };
  bool use_defaults = true;

  // Handle help printing
  if ((argc < 2) && (!use_defaults))
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

  // Create the rate limiter with a default rate.
  if (argc >= 3)
  {
    framerate = std::atof(argv[2]);
  }

  Limiter limiter{ framerate };

  // Try to connect to the provided serial port.
  if (!lights.connect(path))
  {
    std::cout << "Failed to connect to " << path << std::endl;
    return 1;
  }

  const size_t distance_between_sample_pixels { 15 };

  // Create the canvas
  std::vector<RGB> canvas{ lights.ledCount(), { 0, 0, 0 } };
  /*
  while (1)
  {

    limiter.sleep();
    std::vector<RGB> canvasz{ lights.ledCount(), { 200, 200, 200 } };
    lights.write(canvasz);
  }*/

  while (1)
  {
    // Rate limit the loop.
    limiter.sleep();

    // Grab the contents of the screen.
    bool success = sniff->grabContent();
    if (!success)
    {
      std::cerr << "Could not grab desired contents. Quitting." << std::endl;
      return 1;
    }
    const auto image = sniff->getScreen();
    const Box bounds = analyzer.findBorders(*image);
    const auto samplepoints = analyzer.makeBoxSamples(distance_between_sample_pixels, bounds);
    analyzer.sample(*image, bounds, samplepoints, canvas);
    lights.write(canvas);
  }
}
