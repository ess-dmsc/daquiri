#include <core/calibration/parameter.h>
#include <fmt/format.h>
#include <iomanip>
#include <numeric>

#include <core/util/custom_logger.h>

namespace DAQuiri
{

FitParam::FitParam()
  : FitParam(std::numeric_limits<double>::quiet_NaN())
{}

FitParam::FitParam(double v)
  : FitParam(v,
             std::numeric_limits<double>::min(),
             std::numeric_limits<double>::max())
{}

FitParam::FitParam(double v, double lower, double upper)
  : value_(UncertainDouble::from_double(v, std::numeric_limits<double>::quiet_NaN()))
  , lower_(lower)
  , upper_(upper)
{}

FitParam::FitParam(double lower, double upper)
  : FitParam((lower + upper) * 0.5, lower, upper)
{}

void FitParam::set_value(UncertainDouble v)
{
  value_ = v;

  //if out of bounds?
}

void FitParam::set(const FitParam& other)
{
  value_ = other.value_;
  lower_ = other.lower_;
  upper_ = other.upper_;
  enabled_ = other.enabled_;
  fixed_ = other.fixed_;
}

void FitParam::set(double min, double max, double val)
{
  lower_ = min;
  upper_ = max;
  if ((min <= val) && (val <= max))
    value_.setValue(val);
  else
    value_.setValue((min + max)/2.0);
}

void FitParam::preset_bounds(double min, double max)
{
  lower_ = min;
  upper_ = max;
  value_.setValue((min + max)/2.0);
}

void FitParam::constrain(double min, double max)
{
  double l = std::min(min, max);
  double u = std::max(min, max);
  lower_ = std::max(l, lower_);
  upper_ = std::min(u, upper_);
  value_.setValue(std::min(std::max(value_.value(), lower()), upper()));
}

UncertainDouble FitParam::value() const
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
  return (value_.finite() &&
          (value_.value() == lower_) &&
          (lower_ == upper_));
}

void FitParam::set_enabled(bool e)
{
  enabled_ = e;
}

FitParam FitParam::enforce_policy()
{
  FitParam ret = *this;
  if (!ret.enabled_) {
    ret.upper_ = ret.lower_;
    ret.lower_ = 0;
    ret.value_.setValue(ret.lower_);
  } else if (ret.fixed_) {
    ret.upper_ = ret.value_.value() + ret.lower_  * 0.01;
    ret.lower_ = ret.value_.value() - ret.lower_  * 0.01;
  }
  return ret;
}

std::string FitParam::to_string() const
{
  return fmt::format("{} [{}:{}]", value_.to_string(), lower_, upper_);
}

bool FitParam::same_bounds_and_policy(const FitParam &other) const
{
  if (lower_ != other.lower_)
    return false;
  if (upper_ != other.upper_)
    return false;
  if (enabled_ != other.enabled_)
    return false;
  if (fixed_ != other.fixed_)
    return false;
  return true;
}

void to_json(json& j, const FitParam& s)
{
  j["enabled"] = s.enabled_;
  j["fixed"] = s.fixed_;
  j["lower"] = s.lower_;
  j["upper"] = s.upper_;
  j["value"] = s.value_;
}

void from_json(const json& j, FitParam& s)
{
  s.enabled_ = j["enabled"];
  s.fixed_ = j["fixed"];
  s.lower_ = j["lower"];
  s.upper_ = j["upper"];
  s.value_ = j["value"];
}

}
