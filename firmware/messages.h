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
#ifndef FIRMWARE_MESSAGES_H
#define FIRMWARE_MESSAGES_H

//  Message definitions
struct RGB
{
  uint8_t R;
  uint8_t G;
  uint8_t B;
  uint32_t toUint32() const
  {
    return (R << 16) | (G << 8) | B;
  }
};

enum MsgType : uint8_t
{
  NOP = 0,
  CONFIG = 1,
  COLOR = 2
};

struct Config
{
  //! If there has been activity, decay won't take place for decay_time_delay_ms milliseconds.
  uint32_t decay_time_delay_ms;  //!< 0 is disabled.

  //! The decay interval, after inactivity the decay will be performed every decay_interval_us microseconds.
  uint32_t decay_interval_us;

  //! The amount of decay that occurs each cycle.
  uint32_t decay_amount;

  float gamma_r;  //!< Gamma for the red channel.
  float gamma_g;  //!< Gamma for the green channel.
  float gamma_b;  //!< Gamma for the blue channel.
};

struct ColorData
{
  static constexpr const size_t leds_per_message{ 19 };
  static constexpr const size_t settings_show_after{ 1 << 0 };
  static constexpr const size_t settings_set_all{ 1 << 1 };
  uint16_t offset;
  uint8_t settings;
  RGB color[leds_per_message];  // takes 12 messages to send 228 bytes
};

struct Message
{
  MsgType type;
  uint8_t _[3];  // padding
  union {
    ColorData color;
    Config config;
    uint8_t raw[60];
  };
};  // exactly 64 bytes long = 1 usb packet.

#endif