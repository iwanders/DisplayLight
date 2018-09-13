
#include <chrono>
#include <fstream>
#include "pixelsniff.h"

int main(int argc, char* argv[])
{
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

  PixelSniffer sniff;
  sniff.populate();
  size_t target_index = 0;
  for (size_t i = 0; i < std::min<size_t>(1500, sniff.windows_.size()); i++)
  {
    const auto& window = sniff.windows_[i];
    for (size_t j = 0; j < window.level; j++)
    {
      std::cout << " ";
    }
    std::cout << i << " " << window.name << " " << window.width << "x" << window.height;
    auto wmclass = window.window_info.find("WM_CLASS");
    if (wmclass != window.window_info.end())
    {
      std::cout << " " << (*wmclass).second << std::endl;
    }
    else
    {
      std::cout << std::endl;
    }
    for (const auto& it : window.window_info)
    {
    }
  }
  // target_index = 199;

  // target_index++;
  std::cout << "Grabbing: " << target_index << std::endl;
  sniff.selectWindow(target_index);
  bool res;
  size_t count = 500;
  size_t cumulative = 0;
  for (size_t c = 0; c < count; c++)
  {
    tic();
    res = sniff.grabContent();
    if (!res)
    {
      std::cerr << "Failed to grab" << std::endl;
    }
    cumulative += toc();
  }
  std::cout << "iters done:" << count << " avg: " << double(cumulative) / count << " usec" << std::endl;

  std::cout << "Grab result: " << res << std::endl;
  std::cout << "Width: " << sniff.imageWidth() << std::endl;
  std::cout << "Height: " << sniff.imageHeight() << std::endl;

  auto content = sniff.content();
  std::ofstream outcontent("content.ppm");
  outcontent << sniff.imageToPPM(content);
  outcontent.close();
}
