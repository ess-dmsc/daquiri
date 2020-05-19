#pragma once

#include <string>
#include <limits>
#include <nlohmann/json.hpp>

namespace DAQuiri
{

class Parameter
{
 public:
  Parameter() = default;
  Parameter(double v);
  Parameter(double v1, double v2, double v3);
  Parameter(double bound1, double bound2);

  double value() const;
  double lower() const;
  double upper() const;
  bool fixed() const;
  bool implicitly_fixed() const;

  void value(double v);
  void set(double v1, double v2, double v3);
  void constrain(double v1, double v2);

  bool equal_bounds(const Parameter& other) const;

  bool operator==(const Parameter& other) const
  {
    return ((value_ == other.value_)
        && (lower_ == other.lower_)
        && (upper_ == other.upper_));
  }

  bool operator!=(const Parameter& other) const
  {
    return !operator==(other);
  }

  bool operator<(const Parameter& other) const
  {
    if (value_ < other.value_)
      return true;
    if (lower_ < other.lower_)
      return true;
    if (upper_ < other.upper_)
      return true;
    return false;
  }

  std::string to_string() const;

  friend void to_json(nlohmann::json& j, const Parameter& s);
  friend void from_json(const nlohmann::json& j, Parameter& s);

  // \todo static is_finite()

 private:
  double value_{std::numeric_limits<double>::quiet_NaN()};
  double lower_{-1 * std::numeric_limits<double>::infinity()};
  double upper_{std::numeric_limits<double>::infinity()};
  bool fixed_{false};
};

}
