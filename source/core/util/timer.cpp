#include <cmath>
#include <core/util/timer.h>
#include <thread>

void Timer::wait_s(double secs) {
  wait_us(static_cast<uint64_t>(secs * 1000000));
}

void Timer::wait_ms(double msecs) {
  wait_us(static_cast<uint64_t>(msecs * 1000));
}

void Timer::wait_us(uint64_t microsex) {
  std::this_thread::sleep_for(std::chrono::microseconds(microsex));
}

Timer::Timer(bool start) {
  if (start)
    restart();
}

Timer::Timer(double timeout, bool start) {
  timeout_limit = timeout;
  if (start)
    restart();
}

void Timer::restart() { t1 = HRClock::now(); }

double Timer::s() const { return us() * 0.000001; }

double Timer::ms() const { return us() * 0.001; }

double Timer::us() const {
  TP t2 = HRClock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

bool Timer::timeout() const {
  if (0.0 == timeout_limit)
    return false;
  return (s() > timeout_limit);
}

std::string Timer::elapsed_str() const {
  uint64_t e_tot = static_cast<uint64_t>(s());
  uint64_t e_h = e_tot / 3600;
  uint64_t e_m = (e_tot % 3600) / 60;
  uint64_t e_s = (e_tot % 60);

  std::string answer = std::to_string(e_h) + ":";
  if (e_m < 10)
    answer += "0";
  answer += std::to_string(e_m) + ":";
  if (e_s < 10)
    answer += "0";
  answer += std::to_string(e_s);

  return answer;
}

std::string Timer::ETA_str() const {
  uint64_t ETA_tot =
      static_cast<uint64_t>(ceil(static_cast<double>(timeout_limit) - s()));
  uint64_t ETA_h = ETA_tot / 3600;
  uint64_t ETA_m = (ETA_tot % 3600) / 60;
  uint64_t ETA_s = (ETA_tot % 60);

  std::string answer = std::to_string(ETA_h) + ":";
  if (ETA_m < 10)
    answer += "0";
  answer += std::to_string(ETA_m) + ":";
  if (ETA_s < 10)
    answer += "0";
  answer += std::to_string(ETA_s);

  return answer;
}
