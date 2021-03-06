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
#ifndef LIGHTS_H
#define LIGHTS_H

#include <boost/asio.hpp>
#include "../firmware/messages.h"
#include "box.h"

/**
 * @brief This class represents the hardware. It both provides information about where each LED is positioned and which
 *        section each led should represent given some area. It also serves as the interface to the hardware via the
 *        serial port.
 * @note This class contains all the hardware specific values.
 */
class Lights
{
public:
  /**
   * @brief Connect to the serial port.
   * @param serial_path The path to the serial port, usually /dev/ttyACM* or /dev/ttyUSB*
   * @param baudrate The baudrate to use for communication.
   * @return true on success, false in case an error occured.
   */
  bool connect(const std::string& serial_path, size_t baudrate = 115200);

  /**
   * @brief Return the total number of LEDs present.
   */
  static constexpr size_t ledCount()
  {
    return led_count_;
  }

  /**
   * @brief Write a canvas to the leds.
   * @param canvas the canvas to write to the leds. The limiter will be applied to this.
   */
  void write(const std::vector<RGB>& canvas) const;

  /**
   * @brief Set the factor to be used by the limiter. All color channels are multiplied by this factor before being
   *        sent to the leds.
   */
  void setLimitFactor(double factor);

  /**
   * @brief Fill the leds with a certain color.
   */
  void fill(const RGB v = { 0, 0, 0 }) const;

  /**
   * @brief Set a dummy canvas that colors the end of each segment on the leds.
   */
  void writeBoundsCanvas() const;

  /**
   * @brief Create the box that's associated to each led for an rectangle of an arbritrary dimension.
   * @param width The width of the region the boxes should span.
   * @param height The height of the region the boxes should span.
   * @param horizontal_depth The horizontal depth of the cells on the left and right border.
   * @param vertical_depth The vertical depth of the cells on the top and bottom border.
   */
  static std::vector<Box> getBoxes(size_t width, size_t height, size_t horizontal_depth, size_t vertical_depth);

  /**
   * @brief Make a canvas of the appropriate size.
   */
  static std::vector<RGB> makeCanvas();

private:
  static constexpr const size_t horizontal_count_{ 42 };  //!< Number of cells in horizontal direction.
  static constexpr const size_t vertical_count_{ 73 };    //!< Number of cells in vertical direction.
  static constexpr const size_t led_count_{ 228 };        //!< The number of leds in total.

  /**
   * @brief Internal helper function that converts a canvas in a list of messages that can be sent to the serial port.
   */
  std::vector<Message> chunker(const std::vector<RGB>& canvas) const;

  /**
   * @brief Apply the limiter to an RGB tuple.
   */
  void limiter(RGB& rgb) const;

  std::unique_ptr<boost::asio::serial_port> serial_;  //!< Object to interact with the serial port.
  boost::asio::io_service io_;                        //!< IO service.
  double limit_factor_{ 0.5 };                        //!< Limiter multiplication factor.
};

#endif