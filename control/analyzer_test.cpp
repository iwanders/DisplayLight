
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

  if (argc < 2)
  {
    std::cout << "./" << argv[0] << " capture filename" << std::endl;
    std::cout << "./" << argv[0] << " analyze content_in ppm_out" << std::endl;
    std::cout << "./" << argv[0] << " process content_out ppm_out" << std::endl;
    return 1;
  }
  PixelSniffer::Screen content;

  if ((std::string(argv[1]) == "capture") || (std::string(argv[1]) == "process"))
  {
    size_t target_index = 0;
    sniff.selectWindow(0);
    bool res = sniff.grabContent();
    std::cout << "Grab result: " << res << std::endl;
    std::cout << "Width: " << sniff.imageWidth() << std::endl;
    std::cout << "Height: " << sniff.imageHeight() << std::endl;

    content = sniff.content();
    sniff.writeContents(argv[2], content);
  }

  if ((std::string(argv[1]) == "analyze") || (std::string(argv[1]) == "process"))
  {
    content = sniff.readContents(argv[2]);
    auto analyzed = analyzer.sampleCanvas(content, 8);

    for (const auto& sample : analyzed)
    {
      content[sample.y][sample.x] = 0x00FF0000;
    }
    std::ofstream outcontent(argv[3]);
    outcontent << sniff.imageToPPM(content);
    outcontent.close();
  }
}
