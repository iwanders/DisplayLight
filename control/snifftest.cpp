
#include <chrono>
#include <fstream>
#include "pixelsniff.h"

int main(int argc, char* argv[])
{
  // Some helper functions for timing.
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

  PixelSniffer sniff;
  sniff.connect();

  if (argc < 2)
  {
    std::cout << "" << argv[0] << " benchmark" << std::endl;
    std::cout << "" << argv[0] << " grabwindow string_in_title [content.ppm]" << std::endl;
    std::cout << "" << argv[0] << " grabpart string_in_title [content.ppm] x y w h " << std::endl;
    return 1;
  }

  // Benchmark capturing the root window.
  if ((std::string(argv[1]) == "benchmark"))
  {
    sniff.selectRootWindow();
    size_t count = 1000;
    size_t cumulative = 0;
    for (size_t c = 0; c < count; c++)
    {
      tic();
      bool res = sniff.grabContent();
      if (!res)
      {
        std::cerr << "Failed to grab" << std::endl;
      }
      cumulative += toc();
    }
    std::cout << "Captures done:" << count << " avg: " << double(cumulative) / count << " usec" << std::endl;
  }

  // Try grabbing just one window.
  if ((std::string(argv[1]) == "grabwindow") || (std::string(argv[1]) == "grabpart"))
  {
    std::string needle = std::string(argv[2]);
    bool found_window = false;
    for (const auto& window: sniff.getWindows())
    {
      if (window.name.find(needle) != std::string::npos)
      {
        std::cout << "Found window matching: " << needle << std::endl;
        std::cout << "  title: " << window.name  << " " << window.width << ", " << window.height << std::endl;
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
    outcontent << content.imageToPPM();
    outcontent.close();
  }
}
