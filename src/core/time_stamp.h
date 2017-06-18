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

  inline TimeStamp(uint64_t native_time, TimeBase tb)
    : native_(native_time)
    , timebase_(tb)
  {}

  inline TimeStamp make(uint64_t native) const
  {
    TimeStamp ret = *this;
    ret.native_ = native;
    return ret;
  }

  inline uint64_t native() const
  {
    return native_;
  }

  inline double to_nanosec() const
  {
    return timebase_.to_nanosec(native_);
  }

  inline bool same_base(const TimeStamp& other) const
  {
    return (timebase_ == other.timebase_);
  }


  inline int64_t to_native(double ns)
  {
    if (ns > 0)
      return std::ceil(timebase_.to_native(ns));
    //negative?
    return 0;
  }

  inline void delay(double ns)
  {
    if (ns > 0)
      native_ += to_native(ns);
  }

  inline double operator-(const TimeStamp& other) const
  {
    return (to_nanosec() - other.to_nanosec());
  }

  inline bool operator<(const TimeStamp& other) const
  {
    if (same_base((other)))
      return (native_ < other.native_);
    else
      return (to_nanosec() < other.to_nanosec());
  }

  inline bool operator>(const TimeStamp& other) const
  {
    if (same_base((other)))
      return (native_ > other.native_);
    else
      return (to_nanosec() > other.to_nanosec());
  }

  inline bool operator<=(const TimeStamp& other) const
  {
    if (same_base((other)))
      return (native_ <= other.native_);
    else
      return (to_nanosec() <= other.to_nanosec());
  }

  inline bool operator>=(const TimeStamp& other) const
  {
    if (same_base((other)))
      return (native_ >= other.native_);
    else
      return (to_nanosec() >= other.to_nanosec());
  }

  inline bool operator==(const TimeStamp& other) const
  {
    if (same_base((other)))
      return (native_ == other.native_);
    else
      return (to_nanosec() == other.to_nanosec());
  }

  inline bool operator!=(const TimeStamp& other) const
  {
    if (same_base((other)))
      return (native_ != other.native_);
    else
      return (to_nanosec() != other.to_nanosec());
  }

  inline std::string to_string() const
  {
    std::stringstream ss;
    ss << native_ << timebase_.to_string();
    return ss.str();
  }
};

inline void to_json(json& j, const TimeStamp& t)
{
  //do something
}

inline void from_json(const json& j, TimeStamp& t)
{
  //do something
}

}
