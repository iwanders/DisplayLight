#ifndef PIXELSNIFF_H
#define PIXELSNIFF_H

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xproto.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

// For shared memory extension
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>


#include "backed_screen.h"

/**
 * @brief Holds information about one specific window on the X11 desktop.
 */
struct WindowInfo
{
  WindowInfo();
  /**
   * @brief Constructor that retrieves relevant information about the window.
   */
  WindowInfo(Display* display, Window window, size_t level);

  Window window;  //!< X window.
  std::string name;  //!< Window name.
  size_t level;  //!< level of recursion this window was found at.


  Display* display;  //!< Pointer to the current display.
  std::map<std::string, std::string> window_info;  //!< String representation of the windows' properties.

  void getResolution();  //!< Retrieve the windows resolution.
  size_t width;  //!< Window width
  size_t height; //!< Window height
};


class PixelSniffer
{
public:
  PixelSniffer();


  /**
   * @brief Connect the X context, get display pointer. Check if the shared memory extension is present.
   */
  void connect();

  /**
   * @brief Select a window from the window list.
   * @return False if an error occured.
   */
  bool selectWindow(size_t index);
  bool selectWindow(const WindowInfo& window);

  /**
   * @brief Grab a snapshot of the windows' context.
   */
  bool grabContent();

  void setupCaptureArea(size_t x, size_t y, size_t width, size_t height);

  /**
   * @brief Get the current image width.
   */
  size_t imageWidth() const;

  /**
   * @brief Get the current image height.
   */
  size_t imageHeight() const;

  /**
   * @brief Retrieve one pixel from the current image.
   */
  uint32_t imagePixel(size_t x, size_t y);

  /**
   * @brief Return a BackedScreen instance that is backed by the current image in the pixelsniffer.
   */
  BackedScreen getScreen() const;

  /**
   * @brief Populate the window list.
   */
  void populate();

  /**
   * @brief Return the current list of windows that are known.
   */
  std::vector<WindowInfo> getWindows() const;

protected:
  Display* display_;  //!< Pointer to the current X display.
  std::shared_ptr<XImage> ximage_;  //!< Pointer to ximage representing data.
  XShmSegmentInfo shminfo_;  //!< Context for the shared memory extnesion.
  Window root_window_;  //!< The root window of the X display.

  WindowInfo window_;  //!< The current window we are grabbing from.

  size_t x_, y_, width_, height_;

  std::vector<WindowInfo> windows_;

  /**
   * @brief Function to recurse down the window tree, populating the window information structs.
   */
  static void recurseWindows(Display* display, Window root_window, std::vector<WindowInfo>& window_info, size_t level);

  /**
   * @brief Return a list of the current windows that are active on the desktop.
   */
  static std::vector<WindowInfo> recuseWindows(Display* display, Window root_window);

};

#endif
