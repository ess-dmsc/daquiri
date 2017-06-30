#pragma once

#include <string>
#include <cmath>
#include "time_base.h"

namespace DAQuiri {

class TimeStamp
{
private:
  uint64_t native_ {0};
  TimeBase timebase_;

public:
  inline TimeStamp() {}

  inline TimeStamp(uint64_t native_time, TimeBase tb = TimeBase())
    : native_(native_time)
    , timebase_(tb)
  {}

  inline void set_native(uint64_t t)
  {
    native_ = t;
  }

  inline TimeBase base() const
  {
    return timebase_;
  }

  inline uint64_t native() const
  {
    return native_;
  }

  inline double nanosecs() const
  {
    return timebase_.to_nanosec(native_);
  }

  inline bool same_base(const TimeStamp& other) const
  {
    return (timebase_ == other.timebase_);
  }

  inline void delay(double ns)
  {
    native_ += std::round(timebase_.to_native(ns));
  }

  inline TimeStamp delayed(double ns) const
  {
    TimeStamp ret = *this;
    ret.delay(ns);
    return ret;
  }

  inline double operator-(const TimeStamp& other) const
  {
    // if same base, simplify?
    return (nanosecs() - other.nanosecs());
  }

  inline bool operator<(const TimeStamp& other) const
  {
    if (same_base((other)))
      return (native_ < other.native_);
    return (nanosecs() < other.nanosecs());
  }

  inline bool operator>(const TimeStamp& other) const
  {
    if (same_base((other)))
      return (native_ > other.native_);
    return (nanosecs() > other.nanosecs());
  }

  inline bool operator<=(const TimeStamp& other) const
  {
    if (same_base((other)))
      return (native_ <= other.native_);
    return (nanosecs() <= other.nanosecs());
  }

  inline bool operator>=(const TimeStamp& other) const
  {
    if (same_base((other)))
      return (native_ >= other.native_);
    return (nanosecs() >= other.nanosecs());
  }

  inline bool operator==(const TimeStamp& other) const
  {
    if (same_base((other)))
      return (native_ == other.native_);
    return (nanosecs() == other.nanosecs());
  }

  inline bool operator!=(const TimeStamp& other) const
  {
    if (same_base((other)))
      return (native_ != other.native_);
    return (nanosecs() != other.nanosecs());
  }

  inline std::string debug() const
  {
    return std::to_string(native_) + timebase_.debug();
  }
};

inline void to_json(json& j, const TimeStamp& t)
{
  j["native"] = t.native();
  j["base"] = t.base();
}

inline void from_json(const json& j, TimeStamp& t)
{
  t = TimeStamp(j["native"], j["base"]);
}

}
