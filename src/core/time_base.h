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

#include "json.hpp"
using namespace nlohmann;

namespace DAQuiri {

class TimeBase
{
private:
  double multiplier_ {1};
  double divider_ {1};

public:
  inline TimeBase() {}

  inline TimeBase(uint32_t multiplier, uint32_t divider)
  {
    if (!multiplier)
      throw std::domain_error("multiplier must be non-0");
    if (!divider)
      throw std::domain_error("divider must be non-0");
    multiplier_ = multiplier;
    divider_ =  divider;
    auto GCD = gcd(multiplier_, divider_);
    if (GCD > 1)
    {
      multiplier_ /= GCD;
      divider_ /= GCD;
    }
  }

  inline double multiplier() const
  {
    return multiplier_;
  }

  inline double divider() const
  {
    return divider_;
  }

  inline double to_nanosec(double native) const
  {
    return native * multiplier_ / divider_;
  }

  inline double to_native(double ns)
  {
    return ns * divider_ / multiplier_;
  }

  inline bool operator==(const TimeBase& other) const
  {
    return ((divider_ == other.divider_)
            && (multiplier_ == other.multiplier_));
  }

  inline bool operator!=(const TimeBase& other) const
  {
    return (!operator ==(other));
  }

  static inline TimeBase common(const TimeBase& a, const TimeBase& b)
  {
    if (a == b)
      return a;
    return TimeBase(gcd(a.multiplier_, b.multiplier_),
                    lcm(a.divider_, b.divider_));
  }

  inline std::string to_string() const
  {
    if ((multiplier_ != 1) && (divider_ != 1))
      return "x(" + std::to_string(multiplier_) + "/"
          + std::to_string(divider_) + ")";
    return "";
  }

private:
  static inline uint32_t lcm(uint32_t a, uint32_t b)
  {
    uint32_t m(a), n(b);
    while(m!=n)
    {
      if(m < n)
        m = m + a;
      else
        n = n + b;
    }
    return m;
  }

  static inline uint32_t gcd(uint32_t a, uint32_t b)
  {
    return b == 0 ? a : gcd(b, a % b);
  }
};

inline void to_json(json& j, const TimeBase& t)
{
  j["multiplier"] = t.multiplier();
  j["divider"] = t.divider();
}

inline void from_json(const json& j, TimeBase& t)
{
  t = TimeBase(j["multiplier"], j["divider"]);
}

}
