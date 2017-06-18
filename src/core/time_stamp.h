/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 ******************************************************************************/

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
