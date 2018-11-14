#ifndef SCREEN_H
#define SCREEN_H

#include <vector>
#include <memory>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xproto.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <sys/ipc.h>
#include <sys/shm.h>


class BackedScreen
{
  size_t width_;
  size_t height_;

  bool shared_memory_{ false };

  // bitmap backend
  std::vector<std::vector<uint32_t>> map_;
  
  // shared memory backend.
  std::shared_ptr<XImage> ximage_;
  BackedScreen() = default;
public:
  using Bitmap = std::vector<std::vector<uint32_t>>;

  BackedScreen(std::shared_ptr<XImage> image);

  BackedScreen(Bitmap map);

  void convertToBitmap();

  size_t getWidth() const;
  size_t getHeight() const;

  uint32_t pixel(size_t x, size_t y) const;

  void setPixel(size_t x, size_t y, uint32_t pix);

  void hLine(size_t y, uint32_t pix);
  void vLine(size_t x, uint32_t pix);

  void writeContents(const std::string& filename);
  std::string imageToPPM();

  static BackedScreen readContents(const std::string& filename);

};

#endif