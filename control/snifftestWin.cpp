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
#include <fstream>
#include "pixelsniffWin.h"
#include "timing.h"

int main(int argc, char* argv[])
{
  PixelSnifferWin::Ptr sniffer = std::make_shared<PixelSnifferWin>();

  std::cout << "initAdapter " << std::endl;
  sniffer->connect();
  std::cout << "initOutput " << std::endl;
  sniffer->initOutput();
  std::cout << "initDevice " << std::endl;
  sniffer->initDevice();
  std::cout << "initDuplicator " << std::endl;
  sniffer->initDuplicator();
  std::cout << "printVideoOutput " << std::endl;
  sniffer->printVideoOutput();
  std::cout << "grabContent " << std::endl;
  sniffer->grabContent();
  std::cout << "getScreen " << std::endl;
  sniffer->getScreen();
  auto& sniff = *sniffer;
  sniff.connect();

  if (argc < 2)
  {
    std::cout << "" << argv[0] << " benchmark" << std::endl;
    std::cout << "" << argv[0] << " grab content.ppm" << std::endl;
    std::cout << "" << argv[0] << " grabwindow string_in_title [content.ppm]" << std::endl;
    std::cout << "" << argv[0] << " grabpart string_in_title [content.ppm] x y w h " << std::endl;
    return 1;
  }
  if ((std::string(argv[1]) == "grab"))
  {
    sniff.selectRootWindow();
    bool success = sniff.grabContent();
    if (!success)
    {
      std::cerr << "Failed to grab" << std::endl;
    }
    else
    {
      sniff.getScreen()->writeContents(argv[2]);
    }
    return 0;
  }
  // Benchmark capturing the root window.
  if ((std::string(argv[1]) == "benchmark"))
  {
    sniff.selectRootWindow();
    Measure time;
    size_t count = 1000;
    for (size_t c = 0; c < count; c++)
    {
      time.start();
      bool success = sniff.grabContent();
      if (!success)
      {
        std::cerr << "Failed to grab" << std::endl;
      }
      time.stop();
    }
    std::cout << "Captures done:" << count << " avg: " << time.average() << " usec" << std::endl;
  }
}
