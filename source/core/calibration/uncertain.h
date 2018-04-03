//Rework this according to http://arxiv.org/abs/physics/0306138v1 !!!

#pragma once

#include <string>
#include <list>
#include "json.hpp"
using namespace nlohmann;

namespace DAQuiri
{

class UncertainDouble
{
public:

  UncertainDouble() {}
  UncertainDouble(double val, double sigma, uint16_t sigf);

  static UncertainDouble from_int(int64_t val, double sigma);
  static UncertainDouble from_uint(uint64_t val, double sigma);
  static UncertainDouble from_double(double val, double sigma,
                                     uint16_t sigs_below = 1);

  double value() const;
  double uncertainty() const;
  double error() const;

  uint16_t sigfigs() const;
  uint16_t sigdec() const;

  void setValue(double val);
  void setUncertainty(double sigma);
  void setSigFigs(uint16_t sig);
  void autoSigs(uint16_t sigs_below = 1);
  void constrainSigs(uint16_t max_sigs = 6);

  bool finite() const;

  std::string to_string(bool ommit_tiny = true) const;
  std::string error_percent() const;

  std::string debug() const;

  UncertainDouble & operator*=(const double &other);
  UncertainDouble operator*(const double &other) const;
  UncertainDouble & operator/=(const double &other);
  UncertainDouble operator/(const double &other) const;

  UncertainDouble & operator*=(const UncertainDouble &other);
  UncertainDouble operator*(const UncertainDouble &other) const;
  UncertainDouble & operator/=(const UncertainDouble &other);
  UncertainDouble operator/(const UncertainDouble &other) const;

  UncertainDouble & operator+=(const UncertainDouble &other);
  UncertainDouble operator+(const UncertainDouble &other) const;
  UncertainDouble & operator-=(const UncertainDouble &other);
  UncertainDouble operator-(const UncertainDouble &other) const;

  bool almost (const UncertainDouble &other) const;
  bool operator == (const UncertainDouble &other) const {return value() == other.value();}
  bool operator < (const UncertainDouble &other) const {return value() < other.value();}
  bool operator > (const UncertainDouble &other) const {return value() > other.value();}

  static UncertainDouble average(const std::list<UncertainDouble> &list);

private:
  double value_ {std::numeric_limits<double>::quiet_NaN()};
  double sigma_ {std::numeric_limits<double>::quiet_NaN()};
  uint16_t sigfigs_ {0};

  UncertainDouble& additive_uncert(const UncertainDouble &other);
  UncertainDouble& multipli_uncert(const UncertainDouble &other);
  int exponent() const;
};

void to_json(json& j, const UncertainDouble &s);
void from_json(const json& j, UncertainDouble &s);

}
