#pragma once

#include <string>
#include <chrono>

class Timer
{
 private:
  typedef std::chrono::system_clock HRClock;
  typedef std::chrono::time_point<HRClock> TP;

  double timeout_limit{0};

  TP t1;

 public:

  explicit Timer(bool start = false);
  explicit Timer(double timeout, bool start = false);

  void restart();

  [[nodiscard]] double s() const;
  [[nodiscard]] double ms() const;
  [[nodiscard]] double us() const;

  [[nodiscard]] bool timeout() const;

  [[nodiscard]] std::string elapsed_str() const;

  [[nodiscard]] std::string ETA_str() const;

  static void wait_s(double);
  static void wait_ms(double);
  static void wait_us(uint64_t);
};
