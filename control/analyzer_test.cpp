
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
    std::cout << "./" << argv[0] << " borderbisect content_in ppm_out" << std::endl;
    return 1;
  }

  if ((std::string(argv[1]) == "capture"))
  {
    size_t target_index = 0;
    sniff.selectWindow(0);
    bool res = sniff.grabContent();
    auto screen = sniff.getScreen();
    screen.writeContents(argv[2]);
  }


  if (std::string(argv[1]) == "borderbisect")
  {
    auto screen = BackedScreen::readContents(argv[2]);
    //  auto screen = BackedScreen{content};
    size_t x_min;
    size_t y_min;
    size_t x_max;
    size_t y_max;
    analyzer.findBorders(screen, x_min, y_min, x_max, y_max);
    std::cout << "x_min: " << x_min << std::endl;
    std::cout << "y_min: " << y_min << std::endl;
    std::cout << "x_max: " << x_max << std::endl;
    std::cout << "y_max: " << y_max << std::endl;

    auto box_points = analyzer.makeBoxedSamplePoints(15, x_min, y_min, x_max, y_max);

    std::vector<RGB> canvas{analyzer.ledCount(), {0, 0, 0}};
    analyzer.sampleBoxedSamples(screen, x_min, y_min, box_points, canvas);

    analyzer.boxColorizer(canvas, screen);

    screen.hLine(y_min, 0x00FF0000);
    screen.hLine(y_max, 0x0000FF00);
    screen.vLine(x_min, 0x000000FF);
    screen.vLine(x_max, 0x00FF00FF);

    std::ofstream outcontent(argv[3]);
    outcontent << screen.imageToPPM();
    outcontent.close();
  }

}
