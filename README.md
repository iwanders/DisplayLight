DisplayLight
============

![Animated gif of DisplayLight](/../master/displaylight.gif "An animated gif of DisplayLight.")

This yet another ambient-light-behind-the-monitor project; it colors the leds mounted behind one's computer monitor with
the colors as seen on the screen.

It's built for speed; the work in the main loop takes 3525 microseconds on average (1920x1200, using 228 led/regions).
This makes it possible to run it at 60 Hz (takes 3.1% of an i7-4770TE core) or whatever your monitor refresh rate is.

Black border detection uses bisection on all four edges of the screen, ensuring that completely black regions on any
edge don't affect the color analysis. After the region of interest is determined the region that each led will represent
is sampled, the samples for each cell are then averaged.

Boost's async io module is used to communicate with the serial port. Cmake is used for building the project's binaries.

The [firmware](firmware) folder holds code for [Teensy][teensy31] based led hardware, that can drive the ws2811 leds
used for this project (using the DMA based [OctoWS2811][octows]).
The microcontroller performs color correction on each color channel using individual gammalookup tables

All hardware-specific code is isolated to [lights.cpp](control/lights.cpp), the other files should be hardware agnostic.
If you are looking for a project you can use without typing code, [Hyperion][hyperion] may be a better option for you.

Linux
-----
This uses the X11, with shared memory (`libx11-dev`). Build using standard cmake commands, for example from the root of
the directory run:
```
mkdir build
cd build
cmake ../control/
make -j8
```

Windows
-------
Windows is also supported through the [Desktop Duplication][desktopdup] API. Performance is identical to that of Linux.
Using the CMake integration in Visual Studio 2017 dit not work. Installing boost through [vcpkg][vcpkg] did not work for
Visual Studio 2019. It is known to work with Visual Studio 2017, then use cmake through the git bash to create the
solution files, which can then be loaded and built. This should require Windows 8 or higher, but it has only been tested
on Windows 10.

```
#!/bin/bash

# You may need to modify this a bit. This assumes it's ran from the root of the repo.
# And assumes that vcpkg is at the same level of the repo.
VCPKG=$(pwd)/../vcpkg/
CMAKE=${VCPKG}/downloads/tools/cmake-3.14.0-windows/cmake-3.14.0-win32-x86/bin/cmake.exe
TOOLCHAIN=${VCPKG}/scripts/buildsystems/vcpkg.cmake

mkdir build/
cd build/
${CMAKE} -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} -DWIN32=on -G "Visual Studio 15 2017 Win64" ../control/

```

License
------
MIT License, see LICENSE.md.

Copyright (c) 2018 Ivor Wanders

[teensy31]: http://www.pjrc.com/teensy/
[hyperion]: https://github.com/hyperion-project/hyperion
[octows]: https://github.com/PaulStoffregen/OctoWS2811
[desktopdup]: https://docs.microsoft.com/en-us/windows/desktop/direct3ddxgi/desktop-dup-api
[vcpkg]: https://github.com/Microsoft/vcpkg
