//Rework this according to http://arxiv.org/abs/physics/0306138v1 !!!

#pragma once

#include <string>
#include <list>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace DAQuiri
{

class UncertainDouble
{
 private:
  double value_{std::numeric_limits<double>::quiet_NaN()};
  double sigma_{std::numeric_limits<double>::quiet_NaN()};
  uint16_t sigfigs_{1};

 public:

  UncertainDouble() = default;
  explicit UncertainDouble(double val, double sigma, uint16_t sigf);
  explicit UncertainDouble(int64_t val, double sigma);
  explicit UncertainDouble(uint64_t val, double sigma);

  static UncertainDouble from_double(double val, double sigma,
                                     uint16_t sigs_below = 1);

  void value(double val);
  void sigma(double sigma);
  void sigfigs(uint16_t sig);

  void deduce_sigfigs(uint16_t sigs_below = 1);
  void constrain_sigfigs(uint16_t max_sigs = 6);

  double value() const;
  double sigma() const;
  uint16_t sigfigs() const;

  int exponent() const;

  uint16_t sigdec() const;

  double error() const;
  std::string error_str() const;

  bool finite() const;

  std::string to_string(bool ommit_tiny = true) const;

  std::string debug() const;

  UncertainDouble& operator*=(const double& other);
  UncertainDouble operator*(const double& other) const;
  UncertainDouble& operator/=(const double& other);
  UncertainDouble operator/(const double& other) const;

  UncertainDouble& operator*=(const UncertainDouble& other);
  UncertainDouble operator*(const UncertainDouble& other) const;
  UncertainDouble& operator/=(const UncertainDouble& other);
  UncertainDouble operator/(const UncertainDouble& other) const;

  UncertainDouble& operator+=(const UncertainDouble& other);
  UncertainDouble operator+(const UncertainDouble& other) const;
  UncertainDouble& operator-=(const UncertainDouble& other);
  UncertainDouble operator-(const UncertainDouble& other) const;

  bool almost(const UncertainDouble& other) const;
  bool operator==(const UncertainDouble& other) const { return value() == other.value(); }
  bool operator<(const UncertainDouble& other) const { return value() < other.value(); }
  bool operator>(const UncertainDouble& other) const { return value() > other.value(); }

  static UncertainDouble average(const std::list<UncertainDouble>& list);

 private:
  UncertainDouble& additive_uncert(const UncertainDouble& other);
  UncertainDouble& multipli_uncert(const UncertainDouble& other);
};

void to_json(json& j, const UncertainDouble& s);
void from_json(const json& j, UncertainDouble& s);

}
