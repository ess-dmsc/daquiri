#pragma once

#include <string>
#include <cstdint>
#include <thread>
#include <cmath>

#include <chrono>
#include <cstdint>

inline static void wait_ms(int64_t millisex)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(millisex));
}

inline static void wait_us(int64_t microsex)
{
  std::this_thread::sleep_for(std::chrono::microseconds(microsex));
}

class CustomTimer
{
private:
  typedef std::chrono::system_clock HRClock;
  typedef std::chrono::time_point<HRClock> TP;

  const double secs = pow(10, 9);
  const double msecs = pow(10, 6);
  const double usecs = pow(10, 3);
  uint64_t timeout_limit;

  TP t1;

public:
  CustomTimer(bool start = false) {if (start) restart(); timeout_limit = 0;}
  CustomTimer(uint64_t timeout, bool start = false) { if (start) restart(); timeout_limit = timeout;}

  void restart() {
    t1 = HRClock::now();
  }

  double s () {return ns() / secs;}
  double ms () {return ns() / msecs;}
  double us () {return ns() / usecs;}
  double ns() {
    TP t2 = HRClock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
  }
  bool timeout() {
    return (s() > static_cast<double>(timeout_limit));
  }

  std::string done() {
    uint64_t e_tot = static_cast<uint64_t>(s());
    uint64_t e_h = e_tot / 3600;
    uint64_t e_m = (e_tot % 3600) / 60;
    uint64_t e_s = (e_tot % 60);

    std::string answer = std::to_string(e_h) + ":";
    if (e_m < 10) answer += "0";
    answer += std::to_string(e_m) + ":";
    if (e_s < 10) answer += "0";
    answer += std::to_string(e_s);

    return answer;
  }
  
  std::string ETA() {
    uint64_t ETA_tot = static_cast<uint64_t>(
        ceil(static_cast<double>(timeout_limit) - s())
                                   );
    uint64_t ETA_h = ETA_tot / 3600;
    uint64_t ETA_m = (ETA_tot % 3600) / 60;
    uint64_t ETA_s = (ETA_tot % 60);

    std::string answer = std::to_string(ETA_h) + ":";
    if (ETA_m < 10) answer += "0";
    answer += std::to_string(ETA_m) + ":";
    if (ETA_s < 10) answer += "0";
    answer += std::to_string(ETA_s);

    return answer;
  }
};
