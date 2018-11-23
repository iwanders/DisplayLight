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

License
------
MIT License, see LICENSE.md.

Copyright (c) 2018 Ivor Wanders

[teensy31]: http://www.pjrc.com/teensy/
[hyperion]: https://github.com/hyperion-project/hyperion
[octows]: https://github.com/PaulStoffregen/OctoWS2811
