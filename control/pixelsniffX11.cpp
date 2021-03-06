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
#include "pixelsniffX11.h"
#include <chrono>
#include <fstream>
#include <sstream>

WindowInfo::WindowInfo(Display* display_in, Window window_in, size_t level_in)
{
  level = level_in;
  display = display_in;
  window = window_in;

  // Retrieve the window name.
  XTextProperty window_name;
  int status = XGetWMName(display, window_in, &window_name);
  if (status && window_name.value)
  {
    name = std::string(reinterpret_cast<const char*>(window_name.value));
  }

  // Retrieve the Xlist properties.
  int atom_length = 0;
  auto properties = XListProperties(display, window, &atom_length);

  // Iterate over all the entries in the xlist, populate the attribute map.
  for (int i = 0; i < atom_length; i++)
  {
    char* name = XGetAtomName(display, properties[i]);

    XTextProperty label;
    int status = XGetTextProperty(display, window, &label, properties[i]);
    if ((name != nullptr) && status)
    {
      // Got a label, add this to the map.
      window_info[std::string(name)] = std::string(reinterpret_cast<const char*>(label.value));
    }
    XFree(name);
    XFree(label.value);
  }
  XFree(properties);         // Clean up the properties correctly.
  XFree(window_name.value);  // Clean up the properties correctly.
  getResolution();
}

void WindowInfo::getResolution()
{
  int x_return, y_return;
  unsigned int width_return, height_return;
  unsigned int border_width_return;
  unsigned int depth_return;
  Window root_return = XDefaultRootWindow(display);
  XGetGeometry(display, window, &root_return, &x_return, &y_return, &width_return, &height_return, &border_width_return,
               &depth_return);  // return code is discarded, can throw BadDrawable.
  width = width_return;
  height = height_return;
}

void PixelSnifferX11::recurseWindows(Display* display, Window root_window, std::vector<WindowInfo>& window_info,
                                     size_t level)
{
  window_info.push_back(WindowInfo(display, root_window, level));  // Add the root window as first entry.
  level++;

  Window* children = nullptr;
  Window parent;
  unsigned int children_count = 0;
  auto status = XQueryTree(display, root_window, &root_window, &parent, &children, &children_count);

  if ((status == 0) || (children_count == 0))
  {
    return;  // either something failed or at leaf.
  }

  // Recurse into all children.
  for (size_t i = 0; i < children_count; i++)
  {
    recurseWindows(display, children[i], window_info, level);
  }
  XFree(children);
}

std::vector<WindowInfo> PixelSnifferX11::recuseWindows(Display* display, Window root_window)
{
  std::vector<WindowInfo> res;
  recurseWindows(display, root_window, res, 0);
  return res;
}

/**
 * @brief Static free function used to register as an X error handler.
 *        this function always throws on an std::runtime_error.
 */
static int handleError(Display* display, XErrorEvent* error)
{
  std::array<char, 4096> buffer;
  XGetErrorText(display, error->error_code, buffer.data(), buffer.size());
  std::string error_str{ buffer.data() };
  throw std::runtime_error(error_str);
}

PixelSnifferX11::PixelSnifferX11()
{
  XSetErrorHandler(handleError);  // Register the error handler.
}

void PixelSnifferX11::connect()
{
  display_ = XOpenDisplay(nullptr);
  root_window_ = XRootWindow(display_, XDefaultScreen(display_));

  if (!XShmQueryExtension(display_))
  {
    throw std::runtime_error("XShmQueryExtension needs to be available.");
  }
}

std::vector<WindowInfo> PixelSnifferX11::getWindows() const
{
  return recuseWindows(display_, root_window_);
}

bool PixelSnifferX11::selectRootWindow()
{
  window_ = root_window_;
  return prepareCapture();  // default to entire screen.
}

bool PixelSnifferX11::selectWindow(const WindowInfo& window_info)
{
  window_ = window_info.window;
  return prepareCapture();  // default to entire window.
}

bool PixelSnifferX11::prepareCapture(size_t x, size_t y, size_t width, size_t height)
{
  // https://tronche.com/gui/x/xlib/window-information/XGetWindowAttributes.html
  // Colormap for a window seems to be different than for the root?
  XWindowAttributes attributes;
  int status = XGetWindowAttributes(display_, window_, &attributes);
  if (!status)
  {
    return false;
  }

  // Handle inputs arguments.
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

  x = std::min<size_t>(x, attributes.width);
  y = std::min<size_t>(y, attributes.height);

  width = std::min<size_t>(width, attributes.width - x);
  height = std::min<size_t>(height, attributes.height - x);

  // Create an XImage we'll write to, this will be reused until this function is called again.
  ximage_ = std::shared_ptr<XImage>(
      XShmCreateImage(display_, attributes.visual, attributes.depth, ZPixmap, 0, &shminfo_, width, height),
      [](auto z) { XDestroyImage(z); });

  // Initialise the shared memory information.
  shminfo_.shmid = shmget(IPC_PRIVATE, ximage_->bytes_per_line * ximage_->height, IPC_CREAT | 0600);
  ximage_->data = static_cast<char*>(shmat(shminfo_.shmid, 0, 0));
  shminfo_.shmaddr = ximage_->data;
  shminfo_.readOnly = false;

  // Attach the shared memory instance.
  if (!XShmAttach(display_, &shminfo_))
  {
    return false;
  }

  // Store x and y offsets for later.
  capture_x_ = x;
  capture_y_ = y;

  return true;
}

bool PixelSnifferX11::grabContent()
{
  // Lets disable these for now; they raise and map the window, giving best opportunity to be able to capture.
  // The root window is always mapped though.
  //  XMapWindow(display_, window_);
  //  XMapRaised(display_, window_);
  try
  {
    XShmGetImage(display_, window_, ximage_.get(), capture_x_, capture_y_, AllPlanes);
  }
  catch (const std::runtime_error& e)
  {
    std::cout << "Caught exception in " << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
    return false;
  }
  return true;
}

Image::Ptr PixelSnifferX11::getScreen()
{
  return std::make_shared<ImageX11>(ximage_);
}

PixelSniffer::Resolution PixelSnifferX11::getFullResolution()
{
  int x_return, y_return;
  unsigned int width_return, height_return;
  unsigned int border_width_return;
  unsigned int depth_return;
  Window root_return = XDefaultRootWindow(display_);
  XGetGeometry(display_, root_window_, &root_return, &x_return, &y_return, &width_return, &height_return, &border_width_return,
               &depth_return);  // return code is discarded, can throw BadDrawable.
  return Resolution(width_return, height_return);
}
