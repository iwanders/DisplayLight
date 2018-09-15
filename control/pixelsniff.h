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



  PixelSniffer();

  void populate();
  void connect();

  bool selectWindow(size_t index);

  bool grabContent();
  void cleanImage();

  void setupCaptureArea(size_t x, size_t y, size_t width, size_t height);

  size_t imageWidth();
  size_t imageHeight();
  uint32_t imagePixel(size_t x, size_t y);

  std::vector<std::vector<uint32_t>> content();
  void content(std::vector<std::vector<uint32_t>>& content);

  std::string imageToPPM();
  static std::string imageToPPM(const std::vector<std::vector<uint32_t>>& raster);
  std::vector<WindowInfo> windows_;
protected:

  Display* display_;  // incomplete type, cannot use .reset() on shared pointer.
  std::shared_ptr<XImage> ximage_;
  XShmSegmentInfo shminfo_;
  Window root_window_;

  Colormap cmap_;
  size_t window_index_;
  WindowInfo window_;
  size_t x_, y_, width_, height_;
};

#endif
