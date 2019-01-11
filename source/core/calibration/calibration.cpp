#include <core/calibration/calibration.h>
#include <core/calibration/coef_function_factory.h>
#include <core/util/string_extensions.h>
#include <core/util/time_extensions.h>
#include <fmt/format.h>

namespace DAQuiri
{

CalibID::CalibID(std::string val,
                 std::string det,
                 std::string unit)
    : value(val)
      , detector(det)
      , units(unit)
{}

bool CalibID::valid() const
{
  return (!detector.empty() ||
      !value.empty() ||
      !units.empty());
}

bool CalibID::compare(const CalibID& other)
{
  if (!other.detector.empty() && (other.detector != detector))
    return false;
  if (!other.value.empty() && (other.value != value))
    return false;
  if (!other.units.empty() && (other.units != units))
    return false;
  return true;
}

bool CalibID::operator==(const CalibID& other) const
{
  return (detector == other.detector)
      && (value == other.value)
      && (units == other.units);
}

bool CalibID::operator!=(const CalibID& other) const
{
  return !operator==(other);
}

std::string CalibID::debug() const
{
  std::string result;
  if (!detector.empty())
    result += "det=" + detector;
  if (!value.empty())
    result += " val=" + value;
  if (!units.empty())
    result += " units=" + units;
  return result;
}

void to_json(nlohmann::json& j, const CalibID& s)
{
  if (s.detector.size())
    j["detector"] = s.detector;
  if (s.value.size())
    j["value"] = s.value;
  if (s.units.size())
    j["units"] = s.units;
}

void from_json(const nlohmann::json& j, CalibID& s)
{
  if (j.count("detector"))
    s.detector = j["detector"];
  if (j.count("value"))
    s.value = j["value"];
  if (j.count("units"))
    s.units = j["units"];
}

Calibration::Calibration(CalibID from, CalibID to)
    : from_(from)
      , to_(to)
{}

bool Calibration::valid() const
{
  // \todo should require to & from
  return (function_ && !function_->coeffs().empty());
}

CalibID Calibration::from() const
{
  return from_;
}

CalibID Calibration::to() const
{
  return to_;
}

hr_time_t Calibration::created() const
{
  return created_;
}

void Calibration::function(const std::string& type,
                           const std::vector<double>& coefs)
{
  function_ = CoefFunctionFactory::singleton().create_type(type);
  if (function_)
  {
    for (size_t i = 0; i < coefs.size(); ++i)
      function_->set_coeff(i, coefs.at(i));
  }
}

void Calibration::function(CoefFunctionPtr f)
{
  function_ = f;
}

CoefFunctionPtr Calibration::function() const
{
  return function_;
}

bool Calibration::shallow_equals(const Calibration& other) const
{
  return ((from_ == other.from_) && (to_ == other.to_));
}

bool Calibration::operator==(const Calibration& other) const
{
  if (!shallow_equals(other))
    return false;
  if (!function_ && !other.function_)
    return true;
  if (!function_ || !other.function_)
    return false;
  return (function_->type() == other.function_->type())
      && (function_->coeffs() == other.function_->coeffs());
}

bool Calibration::operator!=(const Calibration& other) const
{
  return !operator==(other);
}

double Calibration::transform(double chan) const
{
  if (valid())
    return (*function_)(chan);
  return chan;
}

double Calibration::inverse(double val, double e) const
{
  if (valid())
    return function_->inverse(val, e);
  return val;
}

void Calibration::transform_by_ref(std::vector<double>& data) const
{
  if (valid())
    for (auto& d : data)
      d = (*function_)(d);
}

std::vector<double> Calibration::transform(const std::vector<double>& data) const
{
  std::vector<double> ret = data;
  transform_by_ref(ret);
  return ret;
}

std::string Calibration::debug() const
{
  std::string result;
  if (from_.valid())
    result += "[" + from_.debug() + "]";
  if (to_.valid())
    result += "->[" + to_.debug() + "]";
  if (valid())
  {
    result += " created=" + to_simple(created_);
    result += "\n    model=" + function_->debug();
  }
  return result;
}

void to_json(nlohmann::json& j, const Calibration& s)
{
  j["calibration_creation_date"] = to_iso_extended(s.created_);

  if (s.from_.valid())
    j["from"] = s.from_;

  if (s.to_.valid())
    j["to"] = s.to_;

  if (s.function_)
    j["function"] = (*s.function_);
}

void from_json(const nlohmann::json& j, Calibration& s)
{
  s.created_ = from_iso_extended(j["calibration_creation_date"]);

  if (j.count("from"))
    s.from_ = j["from"];

  if (j.count("to"))
    s.to_ = j["to"];

  if (j.count("function"))
    s.function_ = CoefFunctionFactory::singleton().create_from_json(j["function"]);
}

double shift_down(double v, uint16_t bits)
{
  return v / pow(2, bits);
}

double shift_up(double v, uint16_t bits)
{
  return v * pow(2, bits);
}

double shift(double v, int16_t bits)
{
  if (!bits)
    return v;
  else if (bits < 0)
    return shift_down(v, -bits);
  else
    return shift_up(v, bits);
}

void shift_down(std::vector<double>& vec, uint16_t bits)
{
  double factor = pow(2, bits);
  for (auto& v : vec)
    v /= factor;
}

void shift_up(std::vector<double>& vec, uint16_t bits)
{
  double factor = pow(2, bits);
  for (auto& v : vec)
    v *= factor;
}

void shift(std::vector<double>& vec, int16_t bits)
{
  if (!bits)
    return;
  else if (bits < 0)
    shift_down(vec, -bits);
  else
    shift_up(vec, bits);
}

}

//std::string Calibration::coefs_to_string() const
//{
//  if (!valid())
//    return "";
//  std::stringstream dss;
//  for (auto& q : function_->coeffs())
//    dss << q.second.value() << " ";
//  return trim_copy(dss.str());
//}
//
//std::vector<double> Calibration::coefs_from_string(const std::string& coefs)
//{
//  std::stringstream ss(trim_copy(coefs));
//  std::vector<double> templist;
//  while (ss.rdbuf()->in_avail())
//  {
//    double coef;
//    ss >> coef;
//    templist.push_back(coef);
//  }
//  return templist;
//}
