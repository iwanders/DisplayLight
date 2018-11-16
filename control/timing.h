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
#ifndef TIMING_H
#define TIMING_H
#include <chrono>
#include <thread>

/**
 * @brief Limit a loop to a certain rate in hz, sleeps the appropriate amount since last sleep to ensure we leave sleep
 *        at the start of the next period.
 */
struct Limiter
{
private:
  std::chrono::system_clock::time_point work_start_{ std::chrono::system_clock::now() };
  size_t period_us{ 100000 };

public:
  /**
   * @brief Create the rate limiting object.
   * @param hz The rate to limit the loop at.
   */
  Limiter(double hz) : period_us(1e6 / hz)
  {
  }

  /**
   * @brief Perform the sleep necessary to maintain the rate.
   */
  void sleep()
  {
    std::chrono::duration<double, std::micro> work_time = std::chrono::system_clock::now() - work_start_;
    if (work_time.count() < period_us)
    {
      std::chrono::duration<double, std::micro> delta_us(period_us - work_time.count());
      auto delta_us_duration = std::chrono::duration_cast<std::chrono::microseconds>(delta_us);
      std::this_thread::sleep_for(std::chrono::microseconds(delta_us_duration.count()));
    }
    work_start_ = std::chrono::system_clock::now();
  }
};

/**
 * @brief Calculate cumulative time spent and average duration.
 */
struct Measure
{
private:
  std::chrono::steady_clock::time_point start_;
  size_t count_{ 0 };
  std::chrono::duration<double, std::micro> cumulative_{ 0 };

public:
  /**
   * @brief Start time measurement
   */
  void start()
  {
    start_ = std::chrono::steady_clock::now();
  }

  /**
   * @brief Stop time measurement adding the duration since the last start to the accumulated time.
   */
  void stop()
  {
    auto diff = std::chrono::steady_clock::now() - start_;
    cumulative_ += diff;
    count_++;
  }

  /**
   * @brief Return the total time spent between start and stop calls, in microseconds.
   */
  double total() const
  {
    return cumulative_.count();
  }

  /**
   * @brief Return the average time between each start and stop call, in microseconds.
   */
  double average() const
  {
    return total() / count_;
  }
};

#endif