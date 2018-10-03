//Rework this according to http://arxiv.org/abs/physics/0306138v1 !!!

#include <core/calibration/uncertain.h>

#include <iostream>
#include <limits>
#include <cmath>
#include <core/util/lexical_extensions.h>
#include <core/util/UTF_extensions.h>

#include <core/util/custom_logger.h>

namespace DAQuiri
{

UncertainDouble::UncertainDouble(double val, double sigma, uint16_t sigf)
  : value_(val)
  , sigma_(std::abs(sigma))
  , sigfigs_(sigf)
{
}

std::string UncertainDouble::debug() const
{
  std::stringstream ss;
  ss << "[" << value_ << "\u00B1" << sigma_ << " sigs=" << sigfigs_;
  ss << "] ==> " << to_string();
  return ss.str();
}

UncertainDouble UncertainDouble::from_int(int64_t val, double sigma)
{
  return UncertainDouble (double(val), sigma, order_of(val));
}

UncertainDouble UncertainDouble::from_uint(uint64_t val, double sigma)
{
  return UncertainDouble (double(val), sigma, order_of(val));
}

UncertainDouble UncertainDouble::from_double(double val, double sigma,
                                             uint16_t sigs_below)
{
  UncertainDouble ret(val, sigma, 1);
  ret.autoSigs(sigs_below);
  return ret;
}

int UncertainDouble::exponent() const
{
  int orderOfValue = order_of(value_);
  int orderOfUncert= order_of(sigma_);
  int targetOrder = std::max(orderOfValue, orderOfUncert);

  if ((targetOrder > 5) || (targetOrder < -3))
    return targetOrder;
  else
    return 0;
}

void UncertainDouble::autoSigs(uint16_t sigs_below)
{
  if (std::isfinite(sigma_))
  {
    int16_t order1 = order_of(value_);
    int16_t order2 = order_of(sigma_);
    int16_t upper = std::max(order1, order2);
    int16_t lower = std::min(order1, order2);
    sigfigs_ = upper - lower + sigs_below + 1;
  }
  else
    sigfigs_ = 1 + sigs_below;
  if (exponent() && sigfigs_ > 4)
    sigfigs_ = 4;
}

void UncertainDouble::constrainSigs(uint16_t max_sigs)
{
  if (sigfigs_ > max_sigs)
    sigfigs_ = max_sigs;
}

double UncertainDouble::value() const
{
  return value_;
}

double UncertainDouble::uncertainty() const
{
  return sigma_;
}

double UncertainDouble::error() const
{
  if (value_ != 0)
    return std::abs(sigma_ / value_ * 100.0);
  else
    return std::numeric_limits<double>::quiet_NaN();
}

uint16_t UncertainDouble::sigfigs() const
{
  return sigfigs_;
}

uint16_t UncertainDouble::sigdec() const
{
  int orderOfValue = order_of(value_);
  if (sigfigs_ > orderOfValue) {
    return (sigfigs_ - orderOfValue - 1);
  }
  else
    return 0;
}

void UncertainDouble::setValue(double val)
{
  value_ = val;
}

void UncertainDouble::setUncertainty(double sigma)
{
  sigma_ = std::abs(sigma);
}

void UncertainDouble::setSigFigs(uint16_t sig)
{
  sigfigs_ = sig;
  if (!sig)
    sigfigs_ = 1;
}

bool UncertainDouble::finite() const
{
  return (std::isfinite(value_));
}

std::string UncertainDouble::to_string(bool ommit_tiny) const
{
  if (!std::isfinite(value_))
    return "?";

  int orderOfValue = order_of(value_);
  int orderOfUncert= order_of(sigma_);
  int exp = exponent();

  int decimals = 0;
  if (sigfigs_ > (orderOfValue - exp))
    decimals = sigfigs_ - (orderOfValue - exp) - 1;
  else if (sigfigs_ > orderOfValue)
    decimals = sigfigs_ - orderOfValue;

  std::string result;
  if (std::isinf(sigma_))
    result = "~";

  if (ommit_tiny && std::isfinite(sigma_) && ((orderOfValue - orderOfUncert) < -2))
    result = " . ";
  else
    result += to_str_decimals(value_ / pow(10.0, exp), decimals);

  if (std::isfinite(sigma_) && (sigma_ != 0.0))
  {
    std::string uncertstr;
    if (ommit_tiny && ((orderOfValue - orderOfUncert) > 8))
      uncertstr = "-";
    else if ( ((orderOfValue - orderOfUncert) < -2)
         ||((orderOfValue - orderOfUncert) > 6))
      uncertstr = "HUGE"; //error_percent();
    else {
      double unc_shifted = sigma_ / pow(10.0, exp);
      if (!decimals)
        uncertstr = to_str_decimals(unc_shifted, 0);
      else if (unc_shifted < 1.0)
        uncertstr = to_str_decimals(unc_shifted / pow(10.0, -decimals), 0);
      else
        uncertstr = to_str_decimals(unc_shifted, orderOfUncert - exp + decimals );
    }

    if (!uncertstr.empty())
      result += "(" + uncertstr + ")";
  }

  std::string times_ten("\u00D710");
  if (exp)
    result += times_ten + UTF_superscript(exp);

  return result;
}

std::string UncertainDouble::error_percent() const
{
  if (!finite())
    return "?";
  if ((sigma_ == 0.0) || !std::isfinite(sigma_))
    return "-";
  if (value_ == 0)
    return "?";

  double error = std::abs(sigma_ / value_ * 100.0);

  UncertainDouble p(error, 0, 2);
//  DBG( "perror for " << debug() << " is " << p.debug();
  if (p.exponent() != 0)
    return "(" +  p.to_string(false) + ")%";
  else
    return p.to_string(false) + "%";
}

UncertainDouble & UncertainDouble::operator*=(const UncertainDouble &other)
{
  setSigFigs(std::min(sigfigs(), other.sigfigs()));
  setValue(value_ * other.value_);
  if (finite() && other.finite())
    setUncertainty(sqrt(value_*value_*other.sigma_*other.sigma_
                        + other.value_*other.value_*sigma_*sigma_));
  else
    setUncertainty(std::numeric_limits<double>::quiet_NaN());
  return *this;
}

UncertainDouble & UncertainDouble::operator/=(const UncertainDouble &other)
{
  setSigFigs(std::min(sigfigs(), other.sigfigs()));
  setValue(value_ / other.value_);
  if (finite() && other.finite())
    setUncertainty(sqrt(value_*value_*other.sigma_*other.sigma_
                        + other.value_*other.value_*sigma_*sigma_));
  else
    setUncertainty(std::numeric_limits<double>::quiet_NaN());
  return *this;
}

UncertainDouble & UncertainDouble::operator*=(const double &other)
{
  setValue(value_ * other);
  setUncertainty(std::abs(sigma_ * other));
  return *this;
}

UncertainDouble & UncertainDouble::operator/=(const double &other)
{
  setValue(value_ / other);
  setUncertainty(std::abs(sigma_ / other));
  return *this;
}

UncertainDouble& UncertainDouble::additive_uncert(const UncertainDouble &other)
{
  if (finite() && other.finite())
    setUncertainty(sqrt(pow(sigma_,2) + pow(other.sigma_,2)));
  else
    setUncertainty(std::numeric_limits<double>::quiet_NaN());
  return *this;
}


UncertainDouble& UncertainDouble::operator +=(const UncertainDouble &other)
{
  uint16_t sd1 = sigdec();
  uint16_t sd2 = other.sigdec();
  setValue(value_ + other.value_);
  setSigFigs(std::min(sd1, sd2) + order_of(value_) + 1); //what if order negative?

  additive_uncert(other);
  return *this;
}

UncertainDouble& UncertainDouble::operator -=(const UncertainDouble &other)
{
  uint16_t sd1 = sigdec();
  uint16_t sd2 = other.sigdec();
  setValue(value_ - other.value_);
  setSigFigs(std::min(sd1, sd2) + order_of(value_) + 1); //what if order negative?

  additive_uncert(other);
  return *this;
}

UncertainDouble UncertainDouble::operator +(const UncertainDouble &other) const
{
  UncertainDouble result(*this);
  result += other;
  return result;
}

UncertainDouble UncertainDouble::operator -(const UncertainDouble &other) const
{
  UncertainDouble result(*this);
  result -= other;
  return result;
}

UncertainDouble UncertainDouble::operator *(const UncertainDouble &other) const
{
  UncertainDouble result(*this);
  result *= other;
  return result;
}

UncertainDouble UncertainDouble::operator *(const double &other) const
{
  UncertainDouble result(*this);
  result *= other;
  return result;
}

UncertainDouble UncertainDouble::operator /(const UncertainDouble &other) const
{
  UncertainDouble result(*this);
  result /= other;
  return result;
}

UncertainDouble UncertainDouble::operator /(const double &other) const
{
  UncertainDouble result(*this);
  result /= other;
  return result;
}

bool UncertainDouble::almost(const UncertainDouble &other) const
{
  if (value_ == other.value_)
    return true;
  double delta = std::abs(value_ - other.value_);
  if (std::isfinite(sigma_) && (delta <= sigma_))
    return true;
  if (std::isfinite(other.sigma_) && (delta <= other.sigma_))
    return true;
  return false;
}

UncertainDouble UncertainDouble::average(const std::list<UncertainDouble> &list)
{
  if (list.empty())
    return UncertainDouble();

  double sum = 0;
  for (auto& l : list)
    sum += l.value_;
  double avg = (sum) / list.size();
  double min = avg;
  double max = avg;
  for (auto& l : list)
  {
    if (std::isfinite(l.sigma_))
    {
      min = std::min(min, l.value_ - l.sigma_);
      max = std::max(max, l.value_ + l.sigma_);
    }
  }

  UncertainDouble ret((max + min) * 0.5, (max - min) * 0.5, 1);
  return ret;
}

void to_json(json& j, const UncertainDouble &s)
{
  if (!std::isnan(s.value()))
    j["value"] = s.value();
  if (!std::isnan(s.uncertainty()))
    j["sigma"] = s.uncertainty();
  j["sigfigs"] = s.sigfigs();
}

void from_json(const json& j, UncertainDouble &s)
{
  double val = std::numeric_limits<double>::quiet_NaN();
  if (j.count("value") && j["value"].is_number_float())
    val = j["value"];
  double sigma = std::numeric_limits<double>::quiet_NaN();
  if (j.count("sigma") && j["sigma"].is_number_float())
    sigma = j["sigma"];
  double sigfigs = j["sigfigs"];
  s = UncertainDouble(val, sigma, sigfigs);
}

}
