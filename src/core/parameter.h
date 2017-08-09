#pragma once

#include <string>
#include <limits>
#include "uncertain.h"

namespace DAQuiri
{

class FitParam
{
public:
  FitParam();
  FitParam(double v);
  FitParam(double v, double lower, double upper);
  FitParam(double lower, double upper);

  UncertainDouble value() const;
  double lower() const;
  double upper() const;
  bool enabled() const;
  bool fixed() const;
  bool implicitly_fixed() const;

  void set_enabled(bool e);
  void set_value(UncertainDouble v);
  void set(const FitParam& other);
  void set(double min, double max, double val);
  void preset_bounds(double min, double max);
  void constrain(double min, double max);

  FitParam enforce_policy();

  bool same_bounds_and_policy(const FitParam &other) const;

  bool operator == (const FitParam &other) const {return value_ == other.value_;}
  bool operator < (const FitParam &other) const {return value_ < other.value_;}

  std::string to_string() const;

  friend void to_json(json& j, const FitParam &s);
  friend void from_json(const json& j, FitParam &s);

private:
  UncertainDouble value_;
  double lower_ {std::numeric_limits<double>::min()};
  double upper_ {std::numeric_limits<double>::max()};
  bool enabled_ {true};
  bool fixed_ {false};
};

}
