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
#include "pixelsniffX11.h"
#include "timing.h"

int main(int argc, char* argv[])
{
  PixelSnifferX11 sniff;
  sniff.connect();

  if (argc < 2)
  {
    std::cout << "" << argv[0] << " benchmark" << std::endl;
    std::cout << "" << argv[0] << " grabwindow string_in_title [content.ppm]" << std::endl;
    std::cout << "" << argv[0] << " grabroot [content.ppm]" << std::endl;
    std::cout << "" << argv[0] << " grabpart string_in_title [content.ppm] x y w h " << std::endl;
    return 1;
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

  if ((std::string(argv[1]) == "grabroot"))
  {
    sniff.selectRootWindow();
    sniff.grabContent();
    auto content = sniff.getScreen();
    std::string output_ppm = "window.ppm";
    if (argc >= 3)
    {
      output_ppm = argv[2];
    }
    std::ofstream outcontent(output_ppm);
    outcontent << content->imageToPPM();
    outcontent.close();
    return 0;
  }

  // Try grabbing just one window.
  if ((std::string(argv[1]) == "grabwindow") || (std::string(argv[1]) == "grabpart"))
  {
    std::string needle = std::string(argv[2]);
    bool found_window = false;
    for (const auto& window : sniff.getWindows())
    {
      if (window.name.find(needle) != std::string::npos)
      {
        std::cout << "Found window matching: " << needle << std::endl;
        std::cout << "  title: " << window.name << " " << window.width << ", " << window.height << std::endl;
        sniff.selectWindow(window);
        if (std::string(argv[1]) == "grabpart")
        {
          if (argc < 8)
          {
            std::cout << "" << argv[0] << " grabpart string_in_title [content.ppm] x y w h " << std::endl;
            return 1;
          }
          int x = std::atoi(argv[4]);
          int y = std::atoi(argv[5]);
          int w = std::atoi(argv[6]);
          int h = std::atoi(argv[7]);
          sniff.prepareCapture(x, y, w, h);
        }
        found_window = true;
        break;
      }
    }
    if (!found_window)
    {
      std::cout << "Could not find any window with " << needle << " in the name." << std::endl;
      return 1;
    }
    if (!sniff.grabContent())
    {
      std::cerr << "Failed to grab" << std::endl;
    }
    auto content = sniff.getScreen();
    std::string output_ppm = "window.ppm";
    if (argc >= 4)
    {
      output_ppm = argv[3];
    }
    std::ofstream outcontent(output_ppm);
    outcontent << content->imageToPPM();
    outcontent.close();
  }
}
