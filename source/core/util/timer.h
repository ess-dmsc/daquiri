#pragma once

#include <chrono>
#include <string>

class Timer {
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

  std::string elapsed_str() const;

  std::string ETA_str() const;

  static void wait_s(double);
  static void wait_ms(double);
  static void wait_us(uint64_t);
};
