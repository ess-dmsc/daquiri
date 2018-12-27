#pragma once

#include <string>
#include <limits>
#include <nlohmann/json.hpp>

namespace DAQuiri
{

class FitParam
{
 public:
  FitParam() = default;
  FitParam(double v);
  FitParam(double v1, double v2, double v3);
  FitParam(double bound1, double bound2);

  double value() const;
  double lower() const;
  double upper() const;
  bool enabled() const;
  bool fixed() const;
  bool implicitly_fixed() const;

  void enabled(bool e);
  void value(double v);
  void set(double v1, double v2, double v3);
  void constrain(double v1, double v2);

  bool same_bounds(const FitParam& other) const;
  bool same_policy(const FitParam& other) const;

  bool operator==(const FitParam& other) const
  { return value_ == other.value_; }

  bool operator<(const FitParam& other) const
  { return value_ < other.value_; }

  std::string to_string() const;

  friend void to_json(nlohmann::json& j, const FitParam& s);
  friend void from_json(const nlohmann::json& j, FitParam& s);

  // \todo static is_finite()

 private:
  double value_{std::numeric_limits<double>::quiet_NaN()};
  double lower_{std::numeric_limits<double>::min()};
  double upper_{std::numeric_limits<double>::max()};
  bool enabled_{true};
  bool fixed_{false};
};

}
