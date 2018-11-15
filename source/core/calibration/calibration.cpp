#include <core/calibration/calibration.h>
#include <core/util/string_extensions.h>
#include <core/util/time_extensions.h>
#include <fmt/format.h>

namespace DAQuiri {

CalibID::CalibID(std::string val, std::string det,
                 std::string unit)
  : detector(det)
  , value(val)
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

bool CalibID::operator== (const CalibID& other) const
{
  return (detector == other.detector)
      && (value == other.value)
      && (units == other.units);
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


void to_json(json& j, const CalibID &s)
{
  if (s.detector.size())
    j["detector"] = s.detector;
  if (s.value.size())
    j["value"] = s.value;
  if (s.units.size())
    j["units"] = s.units;
}

void from_json(const json& j, CalibID &s)
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

std::string Calibration::model() const
{
  if (function_)
    return function_->type();
  return "undefined";
}

bool Calibration::operator== (const Calibration& other) const
{
  return shallow_equals(other) &&
      (model() == other.model());
}

CalibID Calibration::to() const
{
  return to_;
}

CalibID Calibration::from() const
{
  return from_;
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



hr_time_t Calibration::calib_date() const
{
  return calib_date_;
}

void Calibration::set_function(const std::string& type,
                  const std::vector<double>& coefs)
{
  function_ = CoefFuncFactory::singleton().create_type(type);
  if (function_)
  {
    for (size_t i=0; i < coefs.size(); ++i)
      function_->set_coeff(i, UncertainDouble(coefs.at(i), 0, 6));
  }
}

void Calibration::set_function(std::shared_ptr<CoefFunction> f)
{
  function_ = f;
}

bool Calibration::operator!= (const Calibration& other) const
{
  return !operator==(other);
}

bool Calibration::valid() const
{
  return (function_ && function_->coeff_count());
}

double Calibration::transform(double chan) const
{
  if (valid())
    return function_->eval(chan);
  return chan;
}

double Calibration::inverse_transform(double energy) const
{
  if (valid())
    return function_->eval_inverse(energy);
  return energy;
}

std::string Calibration::fancy_equation(bool with_chi2) const
{
  if (valid())
    return function_->to_UTF8()
        + (with_chi2
           ? fmt::format("chi2={})", function_->chi2())
           : "");
  return "N/A";
}

void Calibration::transform_ref(std::vector<double>& data) const
{
  if (valid())
    for (auto& d : data)
      d = function_->eval(d);
}

std::vector<double> Calibration::transform_copy(const std::vector<double> &data) const
{
  std::vector<double> ret;
  for (auto &q : data)
    ret.push_back(transform(q));
  return ret;
}

std::string Calibration::coefs_to_string() const
{
  if (!valid())
    return "";
  std::stringstream dss;
  for (auto &q : function_->coeffs_consecutive())
    dss << q << " ";
  return trim_copy(dss.str());
}

std::vector<double> Calibration::coefs_from_string(const std::string &coefs)
{
  std::stringstream ss(trim_copy(coefs));
  std::vector<double> templist;
  while (ss.rdbuf()->in_avail())
  {
    double coef;
    ss >> coef;
    templist.push_back(coef);
  }
  return templist;
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
    result += " eqn=" + fancy_equation(true);
    result += " date=" + to_simple(calib_date_);
  }
  return result;
}

void to_json(json& j, const Calibration &s)
{
  j["calibration_creation_date"] = to_iso_extended(s.calib_date_);

  if (s.from_.valid())
    j["from"] = s.from_;

  if (s.to_.valid())
    j["to"] = s.to_;

  if (s.function_)
  {
    j["function"] = (*s.function_);
    j["function"]["type"] = s.function_->type();
  }
}

void from_json(const json& j, Calibration &s)
{
  s.calib_date_ = from_iso_extended(j["calibration_creation_date"]);

  if (j.count("from"))
    s.from_ = j["from"];

  if (j.count("to"))
    s.to_ = j["to"];

  if (j.count("function"))
  {
    std::string type = j["function"]["type"];
    s.function_ = CoefFuncFactory::singleton().create_type(type);
    if (s.function_)
      from_json(j["function"], *s.function_);
  }
}



}
