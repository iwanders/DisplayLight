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

#include "imageWin.h"

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
  D3D_FEATURE_LEVEL_10_1,
  D3D_FEATURE_LEVEL_10_0,
  D3D_FEATURE_LEVEL_9_3,
  };

  uint32_t createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;


  createFlags |= D3D11_CREATE_DEVICE_DEBUG;

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

void PixelSnifferWin::initDuplicator()
{
  // need to convert output to output1.
  IDXGIOutput1* output1;

  HRESULT hr;

  hr = adapter_output_->QueryInterface(__uuidof(IDXGIOutput1),    (void**)&output1);
  if (FAILED(hr))
    throw std::runtime_error("Failed to query IDXGIOutput1");

  IDXGIOutputDuplication* z;
  hr = output1->DuplicateOutput(device_.get(), &z);

  if (FAILED(hr))
    throw std::runtime_error("Failed to duplicate output");

  duplicator_ = releasing(z);
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


bool PixelSnifferWin::grabContent()
{

  DXGI_OUTDUPL_FRAME_INFO info;
  ID3D11Texture2D* frame;
  IDXGIResource* res;
  HRESULT hr;

  if (duplicator_ == nullptr) {
    return false;
  }

  hr = duplicator_->AcquireNextFrame(100, &info, &res);
  if (hr == DXGI_ERROR_ACCESS_LOST) {
    std::cerr << "Los access " << std::endl;
    return false;
  }
  else if (hr == DXGI_ERROR_WAIT_TIMEOUT)
  {
    std::cout << "Wait timeout " << std::endl;
    return true;
  }
  else if (FAILED(hr))
  {
    std::cout << "Acquire next frame failed miserably." << std::endl;
    return false;
  }

  hr = res->QueryInterface(__uuidof(ID3D11Texture2D), (void**) &frame);

  if (FAILED(hr))
  {
    std::cerr << "Failed to query texture" << std::endl;
    duplicator_->ReleaseFrame();
    return false;
  }

  D3D11_TEXTURE2D_DESC tex_desc;
  frame->GetDesc(&tex_desc);

  D3D11_TEXTURE2D_DESC img_desc;

  if ((image_ != nullptr)) // only retrieve data if image exists.
  {
    image_->GetDesc(&img_desc);
  }
  
  if ((image_ == nullptr) || (img_desc.Width != tex_desc.Width) || (img_desc.Height != tex_desc.Height))  // image is different size.
  {
    // Need to make a new image here now...
    /*
    std::cout << "original tex desc:" << std::endl;
    std::cout << "tex_desc.widtH: " << tex_desc.Width << std::endl;
    std::cout << "tex_desc.Height: " << tex_desc.Height << std::endl;
    std::cout << "tex_desc.MipLevels: " << tex_desc.MipLevels << std::endl;
    std::cout << "tex_desc.ArraySize: " << tex_desc.ArraySize << std::endl;
    std::cout << "tex_desc.Format: " << tex_desc.Format << std::endl;
    std::cout << "tex_desc.SampleDesc.Count: " << tex_desc.SampleDesc.Count << std::endl;
    std::cout << "tex_desc.Usage: " << tex_desc.Usage << std::endl;
    std::cout << "tex_desc.CPUAccessFlags: " << tex_desc.CPUAccessFlags << std::endl;

    D3D11_TEXTURE2D_DESC desc;
    desc.Width = tex_desc.Width;
    desc.Height = tex_desc.Height;
    std::cout << "Making image of : " << desc.Width << " x " << desc.Height << std::endl;
    desc.MipLevels = tex_desc.MipLevels;
    desc.ArraySize = tex_desc.ArraySize;
    desc.Format = tex_desc.Format;
    desc.SampleDesc.Count = tex_desc.SampleDesc.Count;

    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;


    ID3D11Texture2D* img;
    // Maybe this needs to be a cpu device?? 
    hr = device_->CreateTexture2D(&desc, nullptr, &img);*/


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





Image::Ptr PixelSnifferWin::getScreen() const
{
  return std::make_shared<ImageWin>(image_);
}

