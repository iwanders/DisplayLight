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
  uint32_t decay_time_delay_ms;  // 0 is disabled.
  uint32_t decay_interval_us;
  uint32_t decay_amount;
};

struct ColorData
{
  uint16_t offset;
  uint8_t settings;
  RGB color[19];  // takes 12 messages to send 228 bytes
};
#define COLOR_SETTINGS_SHOW_AFTER (1<<0)
#define COLOR_SETTINGS_SET_ALL (1<<1)

struct Message
{
  MsgType type;
  uint8_t _[3];  // padding
  union
  {
    ColorData color;
    Config config;
    uint8_t raw[60];
  };
};  // exactly 64 bytes long = 1 usb packet.



#endif