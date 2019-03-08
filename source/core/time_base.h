#pragma once

#include <string>
#include <cmath>
#include <core/plugin/precise_float.h>
#include <core/plugin/plugin.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace DAQuiri {

class TimeBase
{
private:
  PreciseFloat multiplier_ {1};
  PreciseFloat divider_ {1};

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

  inline PreciseFloat multiplier() const
  {
    return multiplier_;
  }

  inline PreciseFloat divider() const
  {
    return divider_;
  }

  inline PreciseFloat to_nanosec(PreciseFloat native) const
  {
    return native * multiplier_ / divider_;
  }

  inline PreciseFloat to_microsec(PreciseFloat native) const
  {
    return native * multiplier_ / divider_ * 0.001;
  }

  inline PreciseFloat to_millisec(PreciseFloat native) const
  {
    return native * multiplier_ / divider_ * 0.000001;
  }

  inline PreciseFloat to_sec(PreciseFloat native) const
  {
    return native * multiplier_ / divider_ * 0.000000001;
  }

  inline PreciseFloat to_native(PreciseFloat ns)
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

  inline std::string debug() const
  {
    std::stringstream ss;
    if ((multiplier_ != 1) && (divider_ != 1))
      ss << "x(" << multiplier_ << "/" << divider_ << ")";
    return ss.str();
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


class TimeBasePlugin : public Plugin
{
  public:
    TimeBasePlugin()
    {
      std::string r{plugin_name()};

      SettingMeta tm(r + "/multiplier", SettingType::integer, "Timebase multiplier");
      tm.set_val("min", 1);
      tm.set_val("units", "ns");
      add_definition(tm);

      SettingMeta td(r + "/divider", SettingType::integer, "Timebase divider");
      td.set_val("min", 1);
      td.set_val("units", "1/ns");
      add_definition(td);

      SettingMeta root(r, SettingType::stem, "Timebase");
      root.set_enum(0, r + "/multiplier");
      root.set_enum(1, r + "/divider");
      add_definition(root);
    }

    TimeBasePlugin(TimeBase tb)
        : TimeBasePlugin()
    {
      timebase_ = tb;
    }

    TimeBase timebase() const
    {
      return timebase_;
    }

    std::string plugin_name() const override { return "TimeBase"; }

    void settings(const Setting& setting) override
    {
      std::string r{plugin_name()};
      uint32_t mult = setting.find({r + "/multiplier"}).get_int();
      uint32_t div = setting.find({r + "/divider"}).get_int();
      timebase_ = TimeBase(mult ? mult : 1, div ? div : 1);
    }

    Setting settings() const override
    {
      std::string r{plugin_name()};
      auto set = get_rich_setting(r);
      set.set(Setting::integer(r + "/multiplier", timebase_.multiplier()));
      set.set(Setting::integer(r + "/divider", timebase_.divider()));
      return set;
    }

  private:
    TimeBase timebase_;
};

}
