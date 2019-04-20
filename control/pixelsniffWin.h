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
#pragma once

#include "pixelsniff.h"
#include "image.h"
#include <windows.h>
#include <Dxgi.h>
#include <D3dcommon.h>
#include <D3d11.h>
#include <Sysinfoapi.h>
#include <memory>

/*
This looks like the ultimate application for https://docs.microsoft.com/en-us/windows/desktop/direct3ddxgi/desktop-dup-api
*/

template <typename T>
std::shared_ptr<T> releasing(T* z)
{
  return std::shared_ptr<T>(z, [](T* z) {z->Release(); });
}

class PixelSnifferWin : public PixelSniffer
{
public:
  using Ptr = std::shared_ptr<PixelSnifferWin>;
  PixelSnifferWin();

  /**
   * @brief Connect the X context, get display pointer. Check if the shared memory extension is present.
   */
  void connect();

  /**
   * @brief Select the root window to capture from.
   * @return False if an error occured.
   */
  bool selectRootWindow();

  /**
   * @brief Grab a snapshot of the capture area.
   */
  bool grabContent() const;

  /**
   * @brief Return a Image instance that is backed by the current image in the pixelsniffer.
   * @return A screen backed by the shared XImage in this class.
   */
  Image getScreen() const;

  /**
   * @brief Prepares the capture area in the window.
   * @param x The x coordinate in the window (starts left)
   * @param y The y coordinate in the window (starts top)
   * @param width The width of segment to receive, if 0 the window width if used.
   * @param height The height of segment to receive, if 0 the window height if used.
   */
  bool prepareCapture(size_t x = 0, size_t y = 0, size_t width = 0, size_t height = 0);

  std::vector<std::shared_ptr<IDXGIOutput>> PixelSnifferWin::enumerateVideoOutputs();
  void printVideoOutput();

  // One, create an adapter (this is the gfx card?)
  void initAdapter(size_t index=0);

  // Two, initialise / store the output from this card.
  void initOutput(size_t index = 0);

  // Three, create a device object and context to work with.
  void initDevice();

  // Four, create the duplicator.
  //void initDuplicator();

protected:
  size_t capture_x_;  //!< The x position to grab from.
  size_t capture_y_;  //!< The y position to grab from.

  // From the graphics card to the monitor output.
  std::shared_ptr<IDXGIFactory1> factory_;
  std::shared_ptr<IDXGIAdapter1> adapter_;
  std::shared_ptr<IDXGIOutput> adapter_output_;
  
  // The d3d device and device context we're going to interact with.
  std::shared_ptr<ID3D11Device> device_;
  std::shared_ptr<ID3D11DeviceContext> device_context_;

};
