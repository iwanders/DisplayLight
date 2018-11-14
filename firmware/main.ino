#include <OctoWS2811.h>
#include "messages.h"

const int ledsInStrip = 228;

DMAMEM int displayMemory[ledsInStrip*6];
int drawingMemory[ledsInStrip*6];

const int led_config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsInStrip, displayMemory, drawingMemory, led_config);

/*
  Only the first strip is used.
  There are 228 leds in total attached to the first strip.
*/
Config config;
elapsedMillis decay_last_event;
elapsedMicros decay_interval;

uint8_t r_gamma[256] = { 0 };
uint8_t g_gamma[256] = { 0 };
uint8_t b_gamma[256] = { 0 };

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
  config.gamma_r = 1.0;
  config.gamma_g = 1.3;
  config.gamma_b = 1.6;

  // start doing stuff.
  leds.begin();
  fill();
  leds.show();
  delay(3000);


  // reset counters.
  decay_last_event = 0;
  decay_interval = 0;

  // Create the gamma tables
  computeGammaTables();
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
      computeGammaTables();
    break;

    case COLOR:
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
          const uint8_t r_corrected = r_gamma[msg.color.color[i].R];
          const uint8_t g_corrected = g_gamma[msg.color.color[i].G];
          const uint8_t b_corrected = b_gamma[msg.color.color[i].B];
          leds.setPixel(i + msg.color.offset, r_corrected, g_corrected, b_corrected);
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

void createGammaTable(uint8_t* table, float gamma)
{
  for (uint32_t i = 0; i < 256; i++)
  {
    table[i] = pow(static_cast<float>(i) / 256.0, gamma) * 256.0 + 0.5;
  }
}

void computeGammaTables()
{
  createGammaTable(r_gamma, config.gamma_r);
  createGammaTable(g_gamma, config.gamma_g);
  createGammaTable(b_gamma, config.gamma_b);
}