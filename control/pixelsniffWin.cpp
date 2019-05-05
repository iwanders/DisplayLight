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
#include "pixelsniffWin.h"
#include <chrono>
#include <exception>
#include <fstream>
#include <sstream>

#include "imageWin.h"

// Behold! Dark magic to solve linker issues.
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
// ^- This ensures we link against 'IDXGIFactory1'

std::vector<std::shared_ptr<IDXGIOutput>> PixelSnifferWin::enumerateVideoOutputs()
{
  // https://docs.microsoft.com/en-us/windows/desktop/api/dxgi/nf-dxgi-idxgiadapter-enumoutputs
  // EnumOutputs first returns the output on which the desktop primary is displayed. This output corresponds with an
  // index of zero. EnumOutputs then returns other outputs.
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

  // Iterate over the adapters.
  while (factory->EnumAdapters1(i, &adapter) == S_OK)
  {
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
    else
    {
      releasing(adapter);
    }
    std::cout << std::endl;
    i++;
  }
}

void PixelSnifferWin::initDevice()
{
  D3D_FEATURE_LEVEL levelUsed = D3D_FEATURE_LEVEL_9_3;
  HRESULT hr = 0;

  const static D3D_FEATURE_LEVEL featureLevels[] = {
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_3,
  };

  uint32_t createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

  // Enable debugging, this creates fancy output in visual studio if you fail...
  // Failing for example by copying into a mapped memory. Without this flag
  // you don't notice that.
  createFlags |= D3D11_CREATE_DEVICE_DEBUG;

  ID3D11Device* z;
  ID3D11DeviceContext* context;

  hr =
      D3D11CreateDevice(adapter_.get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, createFlags, featureLevels,
                        sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &z, &levelUsed, &context);

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

bool PixelSnifferWin::initDuplicator()
{
  // need to convert output to output1.
  IDXGIOutput1* output1;

  HRESULT hr;

  hr = adapter_output_->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);
  if (FAILED(hr))
    throw std::runtime_error("Failed to query IDXGIOutput1");

  // If lost access, we must release the previous duplicator and make a new one.
  duplicator_.reset();
  duplicator_output_.reset();

  IDXGIOutputDuplication* z;
  hr = output1->DuplicateOutput(device_.get(), &z);

  if (E_ACCESSDENIED == hr)
  {
    // full screen security prompt.
    return false;
  }
  if (hr == DXGI_ERROR_SESSION_DISCONNECTED)
  {
    // seems bad?
    return false;
  }

  if (FAILED(hr))
  {
    throw std::runtime_error("Failed to duplicate output");
  }

  duplicator_ = releasing(z);
  duplicator_output_ = releasing(output1);

  // DXGI_OUTDUPL_DESC out_desc;
  // duplicator_->GetDesc(&out_desc);
  // If the data was already in memory we could map it directly... but it is not.
  // std::cout << "Already in mem: " << out_desc.DesktopImageInSystemMemory << " " << std::endl;
  return true;
}

PixelSnifferWin::PixelSnifferWin()
{
}

void PixelSnifferWin::connect()
{
  // default setup... make all these devices.
  initAdapter();
  initOutput();
  initDevice();
  initDuplicator();
}
bool PixelSnifferWin::selectRootWindow()
{
  return true;
}

bool PixelSnifferWin::prepareCapture(size_t x, size_t y, size_t width, size_t height)
{
  return true;
}

bool PixelSnifferWin::grabContent()
{
  if (duplicator_ == nullptr)
  {
    if (!initDuplicator())
    {
      if (image_ != nullptr)
      {
        return true;  // we have something to deliver, just return like we got the image, even though it's an old image
                      // it's up to date.
      }
    }
  }
  DXGI_OUTDUPL_FRAME_INFO info;
  ID3D11Texture2D* frame;
  IDXGIResource* res;
  HRESULT hr;

  hr = duplicator_->AcquireNextFrame(10, &info, &res);

  if (hr == DXGI_ERROR_ACCESS_LOST)
  {
    // This can happen when the resolution changes, or when we the context changes / full screen application
    // or a d3d11 instance starts, in that case we have to recreate the duplicator.
    std::cerr << "Lost access, trying to reclaim." << std::endl;
    initDuplicator();
    return grabContent();  // just try again.
  }
  else if (hr == DXGI_ERROR_WAIT_TIMEOUT)
  {
    // std::cout << "Wait timeout " << std::endl;
    _com_error err(hr);
    LPCTSTR errMsg = err.ErrorMessage();
    // std::cout << "HR: " << errMsg << std::endl;
    duplicator_->ReleaseFrame();
    if (image_ != nullptr)
    {
      return true;  // we have something to deliver, just return like we got the image, even though it's an old image
                    // it's up to date.
    }
    return false;
  }
  else if (FAILED(hr))
  {
    initDuplicator();
    std::cout << "Acquire next frame failed miserably." << std::endl;
    return false;
  }

  hr = res->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&frame);

  if (FAILED(hr))
  {
    std::cerr << "Failed to query texture" << std::endl;
    duplicator_->ReleaseFrame();
    return false;
  }

  D3D11_TEXTURE2D_DESC tex_desc;
  frame->GetDesc(&tex_desc);

  D3D11_TEXTURE2D_DESC img_desc;

  if ((image_ != nullptr))  // only retrieve data if image exists.
  {
    image_->GetDesc(&img_desc);
  }

  if ((image_ == nullptr) || (img_desc.Width != tex_desc.Width) ||
      (img_desc.Height != tex_desc.Height))  // image is different size.
  {
    // Need to make a new image here now...
    ID3D11Texture2D* img;
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = tex_desc.Width;
    desc.Height = tex_desc.Height;
    desc.Format = tex_desc.Format;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    hr = device_->CreateTexture2D(&desc, nullptr, &img);

    _com_error err(hr);
    LPCTSTR errMsg = err.ErrorMessage();
    std::cout << "Image is now: " << img << " Result: " << errMsg << std::endl;
    image_ = releasing(img);
  }

  // Image is now guaranteed to be good. Copy it.
  device_context_->CopyResource(image_.get(), frame);
  duplicator_->ReleaseFrame();

  return true;
}

Image::Ptr PixelSnifferWin::getScreen()
{
  // Need to make a new image here now, because we can't copy into mapped images, so we need to ensure we hand off a
  // fresh image.
  ID3D11Texture2D* img;
  D3D11_TEXTURE2D_DESC desc = {};

  D3D11_TEXTURE2D_DESC tex_desc;
  image_->GetDesc(&tex_desc);

  desc.Width = tex_desc.Width;
  desc.Height = tex_desc.Height;
  desc.Format = tex_desc.Format;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_STAGING;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

  HRESULT hr = device_->CreateTexture2D(&desc, nullptr, &img);

  // finally, copy the data from the buffer image in this object to the image we'll hand off.
  device_context_->CopyResource(img, image_.get());

  auto imgz = releasing(img);
  return std::make_shared<ImageWin>(imgz);
}
