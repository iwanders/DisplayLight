
#include "pixelsniff.h"

WindowInfo::WindowInfo(Display* displayz, Window windowt, size_t levelt)
{
  level = levelt;
  display = displayz;
  window = windowt;
  int atom_length = 0;
  auto properties = XListProperties(display, window, &atom_length);

  // use thse short hand to grab the window name.
  XTextProperty window_name;
  int status = XGetWMName(display, windowt, &window_name);
  if (status && window_name.value)
  {
    name = std::string(reinterpret_cast<const char*>(window_name.value));
  }

  for (size_t i = 0; i < atom_length; i++)
  {
    char* name = nullptr;
    XTextProperty label;
    name = XGetAtomName(display, properties[i]);
    int status = 0;
    status = XGetTextProperty(display, window, &label, properties[i]);
    if ((name) && (status))
    {
      window_info[std::string(name)] = std::string(reinterpret_cast<const char*>(label.value));
    }
  }
  XFree(properties);  // this is populated with a pointer and magic... clean.
  getResolution();
}

WindowInfo::WindowInfo()
{
}

void WindowInfo::getResolution()
{
  int x_return, y_return;
  unsigned int width_return, height_return;
  unsigned int border_width_return;
  unsigned int depth_return;
  Window root_return = XDefaultRootWindow(display);
  const auto status = XGetGeometry(display, window, &root_return, &x_return, &y_return, &width_return, &height_return,
                                   &border_width_return, &depth_return);
  width = width_return;
  height = height_return;
}

void recurseWindows(Display* display, Window root_window, std::vector<WindowInfo>& window_info, size_t level)
{
  window_info.push_back(WindowInfo(display, root_window, level));
  level++;

  Window* children = nullptr;
  Window parent;
  unsigned int children_count = 0;
  auto status = XQueryTree(display, root_window, &root_window, &parent, &children, &children_count);

  if ((status == 0) || (children_count == 0))
  {
    return;  // either something failed or at leaf.
  }

  for (size_t i = 0; i < children_count; i++)
  {
    recurseWindows(display, children[i], window_info, level);
  }
  XFree(children);
}

std::vector<WindowInfo> recuseWindows(Display* display, Window root_window)
{
  std::vector<WindowInfo> res;
  recurseWindows(display, root_window, res, 0);
  return res;
}

void getRoot(Display*& display, Window& root_window)
{
}

// ugh, legacy stuff is legacy.
static int handleError(Display* display, XErrorEvent* error)
{
  std::cout << "error: " << error->error_code << std::endl;
  std::array<char, 10000> buffer;
  int length = 0;
  XGetErrorText(display, error->error_code, buffer.data(), length);
  std::string error_str{ buffer.data() };
  std::cout << "Error: " << error_str << std::endl;
  if ((error->error_code == BadAlloc) || (error->error_code == BadAccess))
  {
    std::cerr << "Baaaaad";
  }
  throw std::runtime_error("Foo");
}

PixelSniffer::PixelSniffer()
{
  XSetErrorHandler(handleError);
}

void PixelSniffer::connect()
{

  display_ = XOpenDisplay(nullptr);
  root_window_ = XRootWindow(display_, XDefaultScreen(display_));
  int dummy, pixmaps_supported;
  
  if (!XShmQueryExtension(display_))
  {
    throw std::runtime_error("XShmQueryExtension needs to be available.");
  }

  cmap_ = DefaultColormap(display_, DefaultScreen(display_));


}

void PixelSniffer::populate()
{
  auto windows = recuseWindows(display_, root_window_);
  for (const auto& window : windows)
  {
    {
      windows_.push_back(window);
    }
  }
}

void PixelSniffer::cleanImage()
{
  ximage_.reset();
}

bool PixelSniffer::selectWindow(size_t index)
{
  if (index >= windows_.size())
  {
    return false;
  }

  auto& window = windows_[index].window;

  // https://tronche.com/gui/x/xlib/window-information/XGetWindowAttributes.html
  // Colormap for a window seems to be different than for the root?
  XWindowAttributes attributes;
  int status = XGetWindowAttributes(display_, window, &attributes);
  if (!status)
  {
    return false;
  }

  ximage_ = std::shared_ptr<XImage>(XShmCreateImage(display_, attributes.visual,
  attributes.depth, ZPixmap, NULL, &shminfo_,
  attributes.width, attributes.height), [](auto z){});

  shminfo_.shmid = shmget(IPC_PRIVATE, ximage_->bytes_per_line * ximage_->height, IPC_CREAT | 0777);
  ximage_->data = static_cast<char*>(shmat(shminfo_.shmid, 0, 0));
  shminfo_.shmaddr = ximage_->data;
  shminfo_.readOnly = False;

  XShmAttach(display_, &shminfo_);

  cmap_ = attributes.colormap;
  window_index_ = index;
  window_ = windows_[index];
  std::cout << "width: " << attributes.width << "heigh: " << attributes.height << std::endl;
  setupCaptureArea(0, 0, attributes.width, attributes.height);
}

void PixelSniffer::setupCaptureArea(size_t x, size_t y, size_t width, size_t height)
{
  x_ = x;
  y_ = y;
  width_ = width;
  height_ = height;
}

bool PixelSniffer::grabContent()
{
  XMapWindow(display_, window_.window);
  XMapRaised(display_, window_.window);
  try
  {
    //  XImage* ximage = nullptr;
    //  XShmGetImage(display_, window_.window, _xImage, _cropLeft, _cropTop, AllPlanes);
    //  XImage* ximage = XGetImage(display_, window_.window, x_, y_, width_, height_, AllPlanes, ZPixmap);
    //  ximage_.reset(ximage, [](auto im){XDestroyImage(im);});
    XShmGetImage(display_, window_.window, ximage_.get(), 0, 0, AllPlanes);
  }
  catch (const std::runtime_error& e)
  {
    std::cout << "Caught exception in " << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
  }
  return bool{ximage_};
}

size_t PixelSniffer::imageWidth()
{
  if (ximage_)
  {
    return ximage_->width;
  }
  return 0;
}
size_t PixelSniffer::imageHeight()
{
  if (ximage_)
  {
    return ximage_->height;
  }
  return 0;
}

uint32_t PixelSniffer::imagePixel(size_t x, size_t y)
{
  XColor color;
  if ((x < imageWidth()) && (y < imageHeight()))
  {
    color.pixel = XGetPixel(ximage_.get(), x, y);

    // this query here is actually not really necessary... its just ARGB
    /*
    XQueryColor(display_, cmap_, &color);
    uint8_t r = (color.red >> 8);
    uint8_t g = (color.green >> 8);
    uint8_t b = (color.blue >> 8);
    */
    const uint8_t& a = *(reinterpret_cast<uint8_t*>(&color.pixel) + 3);
    const uint8_t& r = *(reinterpret_cast<uint8_t*>(&color.pixel) + 2);
    const uint8_t& g = *(reinterpret_cast<uint8_t*>(&color.pixel) + 1);
    const uint8_t& b = *(reinterpret_cast<uint8_t*>(&color.pixel) + 0);

    return (r << 16) + (g << 8) + b;
    // return color.pixel;
  }
  return 0;
}

void PixelSniffer::content(std::vector<std::vector<uint32_t>>& res)
{
  uint8_t* data = reinterpret_cast<uint8_t*>(ximage_->data);
  const uint8_t stride = ximage_->bits_per_pixel / 8;
  const uint32_t width = imageWidth();
  const uint32_t height = imageHeight();
  for (size_t y = 0; y < height; y++)
  {
    for (size_t x = 0; x < width; x++)
    {
      uint32_t value = *reinterpret_cast<uint32_t*>(data + y * width * stride + x * stride);
      res[y][x] = value;
    }
  }
}

std::vector<std::vector<uint32_t>> PixelSniffer::content()
{
  std::vector<std::vector<uint32_t>> res;
  res.resize(imageHeight());
  const uint32_t width = imageWidth();
  const uint32_t height = imageHeight();
  for (size_t y = 0; y < height; y++)
  {
    res[y].resize(width);
  }
  content(res);
  return res;
}

std::string PixelSniffer::imageToPPM(const std::vector<std::vector<uint32_t>>& raster)
{
  std::stringstream ss;
  ss << "P3\n";
  ss << raster[0].size() << " " << raster.size() << "\n";
  ss << "255\n";
  ss << "# data now\n";
  for (size_t y = 0; y < raster.size(); y++)
  {
    for (size_t x = 0; x < raster[y].size(); x++)
    {
      uint32_t color = raster[y][x];
      const int red = (color >> 16) & 0xFF;
      const int green = (color >> 8) & 0xFF;
      const int blue = (color)&0xFF;
      ss << "" << red << " "
         << " " << green << " " << blue << " ";
    }
    ss << "\n";
  }
  return ss.str();
}

std::string PixelSniffer::imageToPPM()
{
  std::stringstream ss;
  ss << "P3\n";
  ss << imageWidth() << " " << imageHeight() << "\n";
  ss << "255\n";
  ss << "# data now\n";
  for (size_t y = 0; y < imageHeight(); y++)
  {
    for (size_t x = 0; x < imageWidth(); x++)
    {
      uint32_t color = imagePixel(x, y);
      const int red = (color >> 16) & 0xFF;
      const int green = (color >> 8) & 0xFF;
      const int blue = (color)&0xFF;
      ss << "" << red << " "
         << " " << green << " " << blue << " ";
    }
    ss << "\n";
  }
  return ss.str();
}