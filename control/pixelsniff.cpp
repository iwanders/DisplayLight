
#include "pixelsniff.h"
#include <fstream>
#include <chrono>

WindowInfo::WindowInfo(Display* display_in, Window window_in, size_t level_in)
{
  level = level_in;
  display = display_in;
  window = window_in;
  int atom_length = 0;
  auto properties = XListProperties(display, window, &atom_length);

  // use thse short hand to grab the window name.
  XTextProperty window_name;
  int status = XGetWMName(display, window_in, &window_name);
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

void PixelSniffer::recurseWindows(Display* display, Window root_window, std::vector<WindowInfo>& window_info, size_t level)
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

std::vector<WindowInfo> PixelSniffer::recuseWindows(Display* display, Window root_window)
{
  std::vector<WindowInfo> res;
  recurseWindows(display, root_window, res, 0);
  return res;
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

std::vector<WindowInfo> PixelSniffer::getWindows() const
{
  return windows_;
}

bool PixelSniffer::selectWindow(size_t index)
{
  if (index >= windows_.size())
  {
    return false;
  }
  return selectWindow(windows_[index]);
}

bool PixelSniffer::selectWindow(const WindowInfo& window_info)
{
  auto& window = window_info.window;
  window_ = window_info;

  return prepareCapture(0, 0, 0, 0);
}

bool PixelSniffer::prepareCapture(size_t x, size_t y, size_t width, size_t height)
{
  // https://tronche.com/gui/x/xlib/window-information/XGetWindowAttributes.html
  // Colormap for a window seems to be different than for the root?
  XWindowAttributes attributes;
  int status = XGetWindowAttributes(display_, window_.window, &attributes);
  if (!status)
  {
    return false;
  }

  if (width == 0)
  {
    width = attributes.width;
  }
  width = std::min<size_t>(width, attributes.width);

  if (height == 0)
  {
    height = attributes.height;
  }
  height = std::min<size_t>(height, attributes.height);

  ximage_ = std::shared_ptr<XImage>(XShmCreateImage(display_, attributes.visual,
    attributes.depth, ZPixmap, NULL, &shminfo_,
    width, height), [](auto z){});

  shminfo_.shmid = shmget(IPC_PRIVATE, ximage_->bytes_per_line * ximage_->height, IPC_CREAT | 0777);
  ximage_->data = static_cast<char*>(shmat(shminfo_.shmid, 0, 0));
  shminfo_.shmaddr = ximage_->data;
  shminfo_.readOnly = False;

  XShmAttach(display_, &shminfo_);

  capture_x_ = x;
  capture_y_ = y;
}

bool PixelSniffer::grabContent()
{
  // Probably don't need to map them...
  //  XMapWindow(display_, window_.window);
  //  XMapRaised(display_, window_.window);
  try
  {
    XShmGetImage(display_, window_.window, ximage_.get(), capture_x_, capture_y_, AllPlanes);
  }
  catch (const std::runtime_error& e)
  {
    std::cout << "Caught exception in " << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
    return false;
  }
  return true;
}

BackedScreen PixelSniffer::getScreen() const
{
  auto screen = BackedScreen{ximage_};
  return screen;
}
