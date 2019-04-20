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
#include "pixelsniffWin.h"
#include <chrono>
#include <fstream>
#include <exception>
#include <sstream>

// Behold! Dark magic to solve linker issues.
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
// ^- This ensures we link against 'IDXGIFactory1'

std::vector<std::shared_ptr<IDXGIOutput>> PixelSnifferWin::enumerateVideoOutputs()
{
   //https://docs.microsoft.com/en-us/windows/desktop/api/dxgi/nf-dxgi-idxgiadapter-enumoutputs
  //EnumOutputs first returns the output on which the desktop primary is displayed. This output corresponds with an index of zero. EnumOutputs then returns other outputs.
  size_t i = 0;
  IDXGIOutput* pOutput;
  std::vector<std::shared_ptr<IDXGIOutput>> vOutputs;
  while (adapter_->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND)
  {
    vOutputs.emplace_back(releasing(pOutput));
    ++i;
  }
  return vOutputs;
}

void PixelSnifferWin::initAdapter(size_t index)
{
    IDXGIFactory1* factory;
    IDXGIAdapter1* adapter;
    HRESULT hr;
    UINT i = 0;

    IID factoryIID = __uuidof(IDXGIFactory1);

    hr = CreateDXGIFactory1(factoryIID, (void**)&factory);
    if (FAILED(hr))
      throw std::runtime_error("Failed to create DXGIFactory");

    while (factory->EnumAdapters1(i, &adapter) == S_OK) {
      DXGI_ADAPTER_DESC desc;
      hr = adapter->GetDesc(&desc);
      std::cout << "Adapter " << i << " has " << desc.DedicatedVideoMemory << " bytes of memory. ";

      if (FAILED(hr))
        throw std::runtime_error("Failed to retrieve description of adapter.");

      if (i == index)
      {
        std::cout << " Selecting this!";
        adapter_ = releasing(adapter);
        factory_ = releasing(factory);
      }
      std::cout <<  std::endl;
      i++;
    }
}

void PixelSnifferWin::initDevice()
{
  D3D_FEATURE_LEVEL levelUsed = D3D_FEATURE_LEVEL_9_3;
  HRESULT hr = 0;

  const static D3D_FEATURE_LEVEL featureLevels[] =
  {
    D3D_FEATURE_LEVEL_11_0,
  };

  uint32_t createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

  ID3D11Device* z;
  ID3D11DeviceContext* context;
  
  hr = D3D11CreateDevice(adapter_.get(), D3D_DRIVER_TYPE_UNKNOWN,
    NULL, createFlags, featureLevels,
    sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL),
    D3D11_SDK_VERSION, &z,
    &levelUsed, &context);

  if (FAILED(hr))
    throw std::runtime_error("Failed to create device");

  device_ = releasing(z);
  device_context_ = releasing(context);
}

void PixelSnifferWin::printVideoOutput()
{
  auto outs = enumerateVideoOutputs();
  for (auto& out : outs)
  {
    DXGI_OUTPUT_DESC z;
    out->GetDesc(&z);
      std::cout << "z: " << z.DeviceName << "  " << z.Monitor << std::endl;
  }
}

void PixelSnifferWin::initOutput(size_t index)
{
  auto z = enumerateVideoOutputs();
  if (index < z.size())
  {
    adapter_output_ = z[index];
  }
}




PixelSnifferWin::PixelSnifferWin()
{
}

void PixelSnifferWin::connect()
{
}
bool PixelSnifferWin::selectRootWindow()
{
  std::cout << "Select from snifwin" << std::endl;
  return true;
}


bool PixelSnifferWin::prepareCapture(size_t x, size_t y, size_t width, size_t height)
{
  capture_x_ = x;
  capture_y_ = y;

  return true;
}

bool PixelSnifferWin::grabContent() const
{
  std::cout << "Grab from snifwin" << std::endl;
  return true;
}

Image PixelSnifferWin::getScreen() const
{
  std::cout << "Getscreen from snifwin" << std::endl;
  auto res = Image::Bitmap{};
HWND desktop = GetDesktopWindow();
HDC desktopHdc = GetDC(desktop);
//COLORREF color = GetPixel(desktopHdc, x, y);
  for (size_t x = 0; x < 50; x++)
  {
    res.push_back({});
    for (size_t y = 0; y < 50; y++)
    {
      std::cout << "." << std::endl;
      res.back().push_back(GetPixel(desktopHdc, x, y));
    }
  }
  auto screen = Image(res);
  return screen;
}

