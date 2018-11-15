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
  WindowInfo() = default;
  /**
   * @brief Constructor that retrieves relevant information about the window.
   */
  WindowInfo(Display* display, Window window, size_t level);

  Window window;  //!< X window.
  std::string name;  //!< Window name.
  size_t level;  //!< level of recursion this window was found at.

  std::map<std::string, std::string> window_info;  //!< String representation of the windows' properties.

  size_t width;  //!< Window width
  size_t height; //!< Window height
  Display* display;  //!< Pointer to the current display.

private:
  void getResolution();  //!< Retrieve the windows resolution.
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
   * @brief Select the root window to capture from.
   * @return False if an error occured.
   */
  bool selectRootWindow();

  /**
   * @brief Select a specific window to capture from, use getWindows() to obtain a list of input arguments.
   * @return False if an error occured.
   */
  bool selectWindow(const WindowInfo& window);

  /**
   * @brief Grab a snapshot of the windows' context.
   */
  bool grabContent() const;

  /**
   * @brief Return a BackedScreen instance that is backed by the current image in the pixelsniffer.
   * @return A screen backed by the shared XImage in this class.
   */
  BackedScreen getScreen() const;

  /**
   * @brief Return the current list of windows that are known.
   */
  std::vector<WindowInfo> getWindows() const;

  /**
   * @brief Prepares the capture area in the window.
   * @param x The x coordinate in the window (starts left)
   * @param y The y coordinate in the window (starts top)
   * @param width The width of segment to receive, if 0 the window width if used.
   * @param height The height of segment to receive, if 0 the window height if used.
   */
  bool prepareCapture(size_t x, size_t y, size_t width, size_t height);

protected:
  Display* display_;  //!< Pointer to the current X display.
  std::shared_ptr<XImage> ximage_;  //!< Pointer to ximage representing data.
  XShmSegmentInfo shminfo_;  //!< Context for the shared memory extnesion.
  Window root_window_;  //!< The root window of the X display.
  Window window_;  //!< The current window we are grabbing from.

  size_t capture_x_; //!< The x position to grab from.
  size_t capture_y_; //!< The y position to grab from.
  
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
