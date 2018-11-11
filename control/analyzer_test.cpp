
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
    std::cout << "./" << argv[0] << " boxpacker content_in ppm_out" << std::endl;
    std::cout << "./" << argv[0] << " borderbisect content_in ppm_out" << std::endl;
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

  /**
  if (std::string(argv[1]) == "boxpacker")
  {
    content = sniff.readContents(argv[2]);
    auto samples = analyzer.sampleCanvas(content, 8);
    auto box_indices = analyzer.boxPacker(content, samples);
    std::vector<RGB> canvas{analyzer.ledCount(), {0, 0, 0}};
    bool res = analyzer.sampledBoxer(samples, box_indices, canvas);
    std::cout << "Result: " << res << std::endl;
    analyzer.boxColorizer(canvas, content);
    
    std::ofstream outcontent(argv[3]);
    outcontent << sniff.imageToPPM(content);
    outcontent.close();
  }*/

  if (std::string(argv[1]) == "borderbisect")
  {
    content = sniff.readContents(argv[2]);
    size_t x_min;
    size_t y_min;
    size_t x_max;
    size_t y_max;
    analyzer.findBorders(content, x_min, y_min, x_max, y_max);
    std::cout << "x_min: " << x_min << std::endl;
    std::cout << "y_min: " << y_min << std::endl;
    std::cout << "x_max: " << x_max << std::endl;
    std::cout << "y_max: " << y_max << std::endl;

    auto box_points = analyzer.makeBoxedSamplePoints(15, x_min, y_min, x_max, y_max);

    std::vector<RGB> canvas{analyzer.ledCount(), {0, 0, 0}};
    analyzer.sampleBoxedSamples(content, x_min, y_min, box_points, canvas);

    analyzer.boxColorizer(canvas, content);
    std::ofstream outcontent(argv[3]);
    outcontent << sniff.imageToPPM(content);
    outcontent.close();
  }

}
