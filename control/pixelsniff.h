#ifndef PIXELSNIFF_H
#define PIXELSNIFF_H

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xproto.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <X11/Xlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>


#include "backed_screen.h"
struct WindowInfo
{
  WindowInfo();
  Window window;
  size_t level;
  std::string name;
  Display* display;
  std::map<std::string, std::string> window_info;
  WindowInfo(Display* display, Window windowt, size_t levelt);
  void getResolution();
  size_t width;
  size_t height;
};

void recurseWindows(Display* display, Window root_window, std::vector<WindowInfo>& window_info, size_t level);
std::vector<WindowInfo> recuseWindows(Display* display, Window root_window);

void getRoot(Display*& display, Window& root_window);

class PixelSniffer
{
public:
  using Screen = std::vector<std::vector<uint32_t>>;



  PixelSniffer();

  void populate();
  void connect();

  bool selectWindow(size_t index);

  bool grabContent();
  void cleanImage();

  void setupCaptureArea(size_t x, size_t y, size_t width, size_t height);

  size_t imageWidth() const;
  size_t imageHeight() const;
  uint32_t imagePixel(size_t x, size_t y);

  std::vector<std::vector<std::uint32_t>> content() const;
  void content(std::vector<std::vector<std::uint32_t>>& content) const;

  std::vector<WindowInfo> windows_;


  BackedScreen getScreen() const;
protected:

  Display* display_;
  std::shared_ptr<XImage> ximage_;
  XShmSegmentInfo shminfo_;
  Window root_window_;

  Colormap cmap_;
  size_t window_index_;
  WindowInfo window_;
  size_t x_, y_, width_, height_;
};

#endif
