#include <core/util/timer.h>
#include <thread>
#include <cmath>

void Timer::wait_ms(int64_t millisex)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(millisex));
}

void Timer::wait_us(int64_t microsex)
{
  std::this_thread::sleep_for(std::chrono::microseconds(microsex));
}

Timer::Timer(bool start)
{
  if (start)
    restart();
}

Timer::Timer(double timeout, bool start)
{
  timeout_limit = timeout;
  if (start)
    restart();
}

void Timer::restart()
{
  t1 = HRClock::now();
}

double Timer::s() const
{
  TP t2 = HRClock::now();
  return std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
}

double Timer::ms() const
{
  TP t2 = HRClock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
}

double Timer::us() const
{
  TP t2 = HRClock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

bool Timer::timeout() const
{
  if (0.0 == timeout_limit)
    return false;
  return (s() > timeout_limit);
}

std::string Timer::done() const
{
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

std::string Timer::ETA() const
{
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

