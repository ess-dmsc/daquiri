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

  Timer(bool start = false);
  Timer(double timeout, bool start = false);

  void restart();

  double s() const;
  double ms() const;
  double us() const;

  bool timeout() const;

  std::string done() const;

  std::string ETA() const;

  static void wait_ms(int64_t millisex);
  static void wait_us(int64_t microsex);
};
