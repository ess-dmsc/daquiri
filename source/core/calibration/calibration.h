#pragma once

#include <core/util/time_extensions.h>
#include <core/calibration/coef_function.h>

#include <type_traits>

namespace DAQuiri
{

struct CalibID
{
  std::string value;
  std::string detector;
  std::string units;

  CalibID() = default;

  CalibID(std::string val,
          std::string det = "",
          std::string unit = "");

  bool valid() const;
  bool operator==(const CalibID& other) const;
  bool operator!=(const CalibID& other) const;
  std::string debug() const;

  friend void to_json(nlohmann::json& j, const CalibID& s);
  friend void from_json(const nlohmann::json& j, CalibID& s);

  bool compare(const CalibID& other);
};

class Calibration
{
 private:
  hr_time_t created_{std::chrono::system_clock::now()};
  CalibID from_, to_;
  std::shared_ptr<CoefFunction> function_;

 public:
  Calibration() = default;
  Calibration(CalibID from, CalibID to);

  bool valid() const;
  CalibID from() const;
  CalibID to() const;
  hr_time_t created() const;

  CoefFunctionPtr function() const;
  void function(CoefFunctionPtr f);
  void function(const std::string& type, const std::vector<double>& coefs);

  bool shallow_equals(const Calibration& other) const;
  bool operator==(const Calibration& other) const;
  bool operator!=(const Calibration& other) const;

  double transform(double) const;
  double inverse(double val, double e) const;
  void transform_by_ref(std::vector<double>&) const;
  std::vector<double> transform(const std::vector<double>& data) const;

  std::string debug() const;

  friend void to_json(nlohmann::json& j, const Calibration& s);
  friend void from_json(const nlohmann::json& j, Calibration& s);
};

//\todo use traits

double shift_down(double v, uint16_t bits);
double shift_up(double v, uint16_t bits);
double shift(double v, int16_t bits);

void shift_down(std::vector<double>& vec, uint16_t bits);
void shift_up(std::vector<double>& vec, uint16_t bits);
void shift(std::vector<double>& vec, int16_t bits);

}
