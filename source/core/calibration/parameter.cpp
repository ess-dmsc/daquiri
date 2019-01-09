#include <core/calibration/parameter.h>
#include <fmt/format.h>
#include <core/util/compare.h>

namespace DAQuiri
{

Parameter::Parameter(double v)
    : value_(v)
{}

Parameter::Parameter(double v1, double v2, double v3)
{
  set(v1, v2, v3);
}

Parameter::Parameter(double v1, double v2)
{
  constrain(v1, v2);
}

void Parameter::value(double v)
{
  value_ = v;

  if (upper_ < v)
    upper_ = v;
  if (lower_ > v)
    lower_ = v;
}

void Parameter::set(double v1, double v2, double v3)
{
  lower_ = min(v1, v2, v3);
  upper_ = max(v1, v2, v3);
  value_ = mid(v1, v2, v3);
}

void Parameter::constrain(double v1, double v2)
{
  lower_ = std::min(v1, v2);
  upper_ = std::max(v1, v2);
  if ((lower_ <= value_) && (value_ <= upper_))
    return;
  value_ = (v1 + v2) / 2.0;
}

double Parameter::value() const
{
  return value_;
}

double Parameter::lower() const
{
  return lower_;
}

double Parameter::upper() const
{
  return upper_;
}

bool Parameter::fixed() const
{
  return fixed_ || implicitly_fixed();
}

bool Parameter::implicitly_fixed() const
{
  return (std::isfinite(value_) &&
      (value_ == lower_) &&
      (lower_ == upper_));
}

std::string Parameter::to_string() const
{
  return fmt::format("{}[{},{}]", value_, lower_, upper_);
}

bool Parameter::equal_bounds(const Parameter& other) const
{
  return (lower_ == other.lower_) && (upper_ == other.upper_);
}

void to_json(nlohmann::json& j, const Parameter& s)
{
  j["fixed"] = s.fixed_;
  j["lower"] = s.lower_;
  j["upper"] = s.upper_;
  j["value"] = s.value_;
}

void from_json(const nlohmann::json& j, Parameter& s)
{
  s.fixed_ = j["fixed"];
  s.lower_ = j["lower"];
  s.upper_ = j["upper"];
  s.value_ = j["value"];
}

}
