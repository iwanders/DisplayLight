
#include <chrono>
#include <fstream>
#include "screen_analyzer.h"
#include "pixelsniff.h"

int main(int argc, char* argv[])
{
  PixelSniffer sniff;
  ScreenAnalyzer analyzer;
  sniff.connect();
  sniff.populate();
  size_t target_index = 0;
 
  sniff.selectWindow(0);
  bool res = sniff.grabContent();
  auto analyzed = analyzer.sampleCanvas(sniff.content());

  std::cout << "Grab result: " << res << std::endl;
  std::cout << "Width: " << sniff.imageWidth() << std::endl;
  std::cout << "Height: " << sniff.imageHeight() << std::endl;

  auto content = sniff.content();
  for (const auto& sample : analyzed)
  {
    std::cout << " y: " << sample.y << " x: " << sample.x << std::endl;
    content[sample.y][sample.x] = 0x00FF0000;
  }
  std::cout << "Saving." << std::endl;
  std::ofstream outcontent("analyzed.ppm");
  outcontent << sniff.imageToPPM(content);
  outcontent.close();
}
