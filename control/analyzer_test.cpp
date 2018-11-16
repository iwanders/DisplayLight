
#include <chrono>
#include <fstream>
#include "analyzer.h"
#include "pixelsniff.h"

int main(int argc, char* argv[])
{
  PixelSniffer sniff;
  Analyzer analyzer;
  sniff.connect();

  if (argc < 2)
  {
    std::cout << "./" << argv[0] << " capture filename" << std::endl;
    std::cout << "./" << argv[0] << " borderbisect content_in ppm_out" << std::endl;
    return 1;
  }

  if ((std::string(argv[1]) == "capture"))
  {
    sniff.selectRootWindow();
    bool res = sniff.grabContent();
    if (!res)
    {
      std::cerr << "Failed to grab content" << std::endl;
      return 1;
    }
    auto screen = sniff.getScreen();
    screen.writeContents(argv[2]);
  }

  if (std::string(argv[1]) == "borderbisect")
  {
    auto screen = Image::readContents(argv[2]);
    //  auto screen = Image{content};
    auto bounds = analyzer.findBorders(screen);
    std::cout << "bounds: " << std::string(bounds) << std::endl;
    auto box_points = analyzer.makeBoxedSamplePoints(15, bounds);

    std::vector<RGB> canvas{analyzer.ledCount(), {0, 0, 0}};
    analyzer.sampleBoxedSamples(screen, bounds, box_points, canvas);

    analyzer.boxColorizer(canvas, screen);

    screen.hLine(bounds.y_min, 0x00FF0000);
    screen.hLine(bounds.y_max, 0x0000FF00);
    screen.vLine(bounds.x_min, 0x000000FF);
    screen.vLine(bounds.x_max, 0x00FF00FF);

    std::ofstream outcontent(argv[3]);
    outcontent << screen.imageToPPM();
    outcontent.close();
  }

}
