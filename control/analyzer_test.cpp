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
#include "analyzer.h"
#include <fstream>
#include "pixelsniff.h"

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cout << "./" << argv[0] << " capture image_out.bin" << std::endl;
    std::cout << "./" << argv[0] << " borderbisect image_in.bin image_out.ppm" << std::endl;
    return 1;
  }

  // Simple capture operation, captures to binary file.
  if ((std::string(argv[1]) == "capture"))
  {
    // Create the pixel sniffer.
    PixelSniffer sniff;
    sniff.connect();
    sniff.selectRootWindow();
    bool success = sniff.grabContent();
    if (!success)
    {
      std::cerr << "Failed to grab content" << std::endl;
      return 1;
    }
    auto image = sniff.getScreen();
    image.writeContents(argv[2]);
  }

  // Test border detection.
  if (std::string(argv[1]) == "borderbisect")
  {
    Analyzer analyzer;

    // Read the file.
    auto image = Image::readContents(argv[2]);

    // Detect the borders
    auto bounds = analyzer.findBorders(image);
    std::cout << "bounds: " << std::string(bounds) << std::endl;

    // Create sample points associated to borders.
    auto box_points = analyzer.makeBoxSamples(15, bounds);

    // Allocate led canvas.
    auto canvas = analyzer.makeCanvas();

    // Perform sample operation on the image, using bounds and box_points. This fills canvas with analyzed colors.
    analyzer.sample(image, bounds, box_points, canvas);

    // Draw the canvas' boxes on the image.
    analyzer.boxColorizer(canvas, image);

    // Draw the borders
    image.hLine(bounds.y_min, 0x00FF0000);
    image.hLine(bounds.y_max, 0x0000FF00);
    image.vLine(bounds.x_min, 0x000000FF);
    image.vLine(bounds.x_max, 0x00FF00FF);

    // Write the analysed results as ppm file.
    std::ofstream outcontent(argv[3]);
    outcontent << image.imageToPPM();
    outcontent.close();
  }

  return 0;
}
