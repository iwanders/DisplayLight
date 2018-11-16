#include <iostream>
#include <vector>

#include "pixelsniff.h"
#include "analyzer.h"
#include "lights.h"
#include "timing.h"

void printHelp(const std::string& progname)
{
    std::cout << "" << progname << " serial_port_path [framerate_in_hz]" << std::endl;
}

int main(int argc , char* argv[])
{
  PixelSniffer sniff;
  sniff.connect();
  sniff.selectRootWindow();

  Lights lights;
  Analyzer analyzer;

  // Handle help printing
  if ((argc < 2) || (std::string(argv[1]) == "--help"))
  {
    printHelp(argv[0]);
    return 1;
  }

  // Create the rate limiter with a default rate.
  double framerate { 60 };
  if (argc >= 3)
  {
    framerate = std::atof(argv[2]);
  }
  Limiter limiter{framerate};

  // Try to connect to the provided serial port.
  if (!lights.connect(argv[1]))
  {
    std::cout << "Failed to connect to " << argv[1] << std::endl;
    return 1;
  }

  // Crate the canvas
  std::vector<RGB> canvas{lights.ledCount(), {0, 0, 0}};

  while (1)
  {
    // Rate limit the loop.
    limiter.sleep();

    // Grab the contents of the screen.
    bool success = sniff.grabContent();
    if (!success)
    {
      std::cerr << "Could not grab desired contents. Quitting." << std::endl;
      return 1;
    }
    const auto image = sniff.getScreen();
    const Box bounds = analyzer.findBorders(image);
    const auto samplepoints = analyzer.makeBoxSamples(15, bounds);
    analyzer.sample(image, bounds, samplepoints, canvas);
    lights.write(canvas);
  }
}
