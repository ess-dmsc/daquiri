#include <core/calibration/parameter.h>
#include <fmt/format.h>
#include <core/util/compare.h>

namespace DAQuiri
{

FitParam::FitParam(double v)
    : value_(v)
{}

FitParam::FitParam(double v1, double v2, double v3)
{
  set(v1, v2, v3);
}

FitParam::FitParam(double v1, double v2)
{
  constrain(v1, v2);
}

void FitParam::value(double v)
{
  value_ = v;

  //if out of bounds?
}

void FitParam::set(double v1, double v2, double v3)
{
  lower_ = min(v1, v2, v3);
  upper_ = max(v1, v2, v3);
  value_ = mid(v1, v2, v3);
}

void FitParam::constrain(double v1, double v2)
{
  lower_ = std::min(v1, v2);
  upper_ = std::max(v1, v2);
  if ((value_ < lower_) || (value_ > upper_))
    value_ = (v1 + v2) / 2.0;
}

double FitParam::value() const
{
  return value_;
}

double FitParam::lower() const
{
  return lower_;
}

double FitParam::upper() const
{
  return upper_;
}

bool FitParam::enabled() const
{
  return enabled_;
}

bool FitParam::fixed() const
{
  return fixed_ || implicitly_fixed();
}

bool FitParam::implicitly_fixed() const
{
  return (std::isfinite(value_) &&
      (value_ == lower_) &&
      (lower_ == upper_));
}

void FitParam::enabled(bool e)
{
  enabled_ = e;
}

std::string FitParam::to_string() const
{
  return fmt::format("{}[{},{}]", value_, lower_, upper_);
}

bool FitParam::same_bounds(const FitParam& other) const
{
  if (lower_ != other.lower_)
    return false;
  if (upper_ != other.upper_)
    return false;
  return true;
}

bool FitParam::same_policy(const FitParam& other) const
{
  if (enabled_ != other.enabled_)
    return false;
  if (fixed_ != other.fixed_)
    return false;
  return true;
}

void to_json(nlohmann::json& j, const FitParam& s)
{
  j["enabled"] = s.enabled_;
  j["fixed"] = s.fixed_;
  j["lower"] = s.lower_;
  j["upper"] = s.upper_;
  j["value"] = s.value_;
}

void from_json(const nlohmann::json& j, FitParam& s)
{
  s.enabled_ = j["enabled"];
  s.fixed_ = j["fixed"];
  s.lower_ = j["lower"];
  s.upper_ = j["upper"];
  s.value_ = j["value"];
}

}
