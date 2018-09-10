#include <OctoWS2811.h>

const int ledsInStrip = 228;

DMAMEM int displayMemory[ledsInStrip*6];
int drawingMemory[ledsInStrip*6];

const int led_config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsInStrip, displayMemory, drawingMemory, led_config);

/*
  Only the first strip is used.
  There are 228 leds in total attached to the first strip.
*/

//  Message definitions
using RGB = struct
{
  uint8_t R;
  uint8_t G;
  uint8_t B;
};

enum MsgType
{
  NOP = 0,
  CONFIG = 1,
  COLORS = 2
};
using Config = struct
{
  uint32_t decay_time_delay_ms;  // 0 is disabled.
  uint32_t decay_interval_us;
  uint32_t decay_amount;
};

using ColorData = struct
{
  uint16_t offset;
  uint8_t settings;
  RGB color[19];  // takes 12 messages to send 228 bytes
};
#define COLOR_SETTINGS_SHOW_AFTER (1<<0)
#define COLOR_SETTINGS_SET_ALL (1<<1)

using Message = struct
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


//

Config config;
elapsedMillis decay_last_event;
elapsedMicros decay_interval;

void fill(int color=0)
{
  for (int j=0; j < leds.numPixels(); j++)
  {
    leds.setPixel(j, color);
  }
}

void setPixel(int i, int color)
{
  fill();
  leds.setPixel(i, color);
  leds.show();
}

void runner()
{
  for (int i = 0; i < ledsInStrip; i++)
  {
    setPixel(i, 0xFFFFFF);
    delayMicroseconds(5000);
  }
}

void setup()
{
  // set defaults.
  config.decay_time_delay_ms = 1000;
  config.decay_interval_us = 1000;
  config.decay_amount = 1;

  // start doing stuff.
  leds.begin();
  fill();
  leds.show();
  delay(3000);

  // reset counters.
  decay_last_event = 0;
  decay_interval = 0;
}

void loopDecay()
{
  if (config.decay_time_delay_ms == 0)
  {
    // Serial.println("decay disabled");
    return;
  }

  // decay enabled
  if (decay_last_event >= config.decay_time_delay_ms)
  {
    if (decay_interval >= config.decay_interval_us)
    {
      for (int j = 0; j < ledsInStrip; j++)
      {
        // Serial.print("j: "); Serial.println(j); delay(10);
        const int c = leds.getPixel(j);

        // Decompose the color
        int16_t R = (c >> 16) & 0xFF;
        int16_t G = (c >> 8) & 0xFF;
        int16_t B = (c >> 0) & 0xFF;

        // Do the decay
        R = (R - config.decay_amount);
        G = (G - config.decay_amount);
        B = (B - config.decay_amount);

        // prevent underflow
        R = (R < 0) ? 0 : R;
        G = (G < 0) ? 0 : G;
        B = (B < 0) ? 0 : B;
  
        leds.setPixel(j, R, G, B);
      }
      leds.show();
      decay_interval = 0;
    }
  }
}

void processCommand(const Message& msg)
{
  switch (msg.type)
  {
    case NOP:
    break;

    case CONFIG:
      config = msg.config;
    break;

    case COLORS:
      if (msg.color.settings & COLOR_SETTINGS_SET_ALL)
      {
        // setting all colors
        fill(leds.color(msg.color.color[0].R, msg.color.color[0].G, msg.color.color[0].B));
      }
      else
      {
        // setting indivual colors
        for (uint16_t i = 0; i < sizeof(msg.color.color) / sizeof(msg.color.color[0]); i++)
        {
          leds.setPixel(i + msg.color.offset, msg.color.color[i].R, msg.color.color[i].G, msg.color.color[i].B);
        }
      }
      // If set, then show the leds.
      if (msg.color.settings & COLOR_SETTINGS_SHOW_AFTER)
      {
        leds.show();
      }
    break;

    default:
    break;
  }
}

void loopSerial()
{
  // Check if we should read the serial port for commands.
  if (Serial.available())
  {
    Message msg;
    if (Serial.readBytes(reinterpret_cast<char*>(&msg), sizeof(msg)) == sizeof(msg))
    {
      decay_last_event = 0;
      processCommand(msg);
    }
  }
}

void loop()
{
  loopDecay();
  loopSerial();
}

