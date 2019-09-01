/*
  The MIT License (MIT)
  Copyright (c) 2019 Ivor Wanders
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

#include <D3d11.h>
#include <D3dcommon.h>
#include <Dxgi.h>
#include <Dxgi1_2.h>
#include <Sysinfoapi.h>
#include <Wincodec.h>
#include <comdef.h>
#include <windows.h>
#include <memory>
#include "image.h"
#include "pixelsniff.h"

/*
Most information taken from the desktop duplication api, which seems to be superbly applicable.
https://docs.microsoft.com/en-us/windows/desktop/direct3ddxgi/desktop-dup-api
*/

/**
 * @brief Helper function to create a shared pointer that cleans up using Microsoft's 'Release()' method.
 */
template <typename T>
std::shared_ptr<T> releasing(T* z)
{
  return std::shared_ptr<T>(z, [](T* z) { z->Release(); });
}

/**
 * @brief PixelSniffer using the Direct 3D DXGI Desktop Duplication api, for Windows (8+?, 10 works...)
 */
class PixelSnifferWin : public PixelSniffer
{
public:
  using Ptr = std::shared_ptr<PixelSnifferWin>;
  PixelSnifferWin();

  /**
   * @brief Create all objects / devices necessary to start grabbing screens.
   */
  void connect();

  /**
   * @brief Just returns true for this implementation.
   */
  bool selectRootWindow();

  /**
   * @brief Grab a snapshot of the capture area.
   */
  bool grabContent();

  /**
   * @brief Return an image copy of the content that was just grabbed.
   */
  Image::Ptr getScreen();

  /**
   * @brief Just returns true for this implementation.
   */
  bool prepareCapture(size_t x = 0, size_t y = 0, size_t width = 0, size_t height = 0);

  /**
   * @brief Step One, create an adapter (this is the gfx card?)
   */
  void initAdapter(size_t index = 0);

  /**
   * @brief Enumerate the video outputs of the created adapter.
   */
  std::vector<std::shared_ptr<IDXGIOutput>> PixelSnifferWin::enumerateVideoOutputs();

  //! Print video output information.
  void printVideoOutput();

  /**
   * @bief Step Two, initialise / store the output from this card.
   */
  void initOutput(size_t index = 0);

  /**
   * @bief Step Three, create a device object and context to work with.
   */
  void initDevice();

  /**
   * @bief Step Four, create the duplicator, if we lose the resource, we need to reinitialise this.
   */
  bool initDuplicator();

  Resolution getFullResolution();

protected:
  // From the graphics card to the monitor output.
  std::shared_ptr<IDXGIFactory1> factory_;
  std::shared_ptr<IDXGIAdapter1> adapter_;
  std::shared_ptr<IDXGIOutput> adapter_output_;

  // The d3d device and device context we're going to interact with.
  std::shared_ptr<ID3D11Device> device_;
  std::shared_ptr<ID3D11DeviceContext> device_context_;

  // For the actual output duplication.
  std::shared_ptr<IDXGIOutputDuplication> duplicator_;
  std::shared_ptr<IDXGIOutput1> duplicator_output_;
  std::shared_ptr<ID3D11Texture2D> image_;
};
