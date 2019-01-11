#pragma once

#include <core/calibration/parameter.h>
#include <map>

namespace DAQuiri
{

class CoefFunction
{
 public:
  CoefFunction() = default;
  virtual ~CoefFunction() = default;
  CoefFunction(const std::vector<double>& coeffs, double uncert, double chisq);

  void x_offset(const Parameter& o);
  Parameter x_offset() const;

  void chi2(double);
  double chi2() const;

  void set_coeff(int degree, const Parameter& p);
  std::map<int, Parameter> coeffs() const;

  std::vector<double> eval(const std::vector<double>& x) const;
  double inverse(double y, double e = 0.1) const;

  //TO IMPLEMENT IN CHILDREN
  virtual std::string type() const = 0;
  virtual CoefFunction* clone() const = 0;

  virtual double operator() (double x) const = 0;
  virtual double derivative(double x) const = 0;

  virtual std::string debug() const = 0;
  virtual std::string to_UTF8(int precision, bool with_rsq) const = 0;
  virtual std::string to_markup(int precision, bool with_rsq) const = 0;


 protected:
  std::map<int, Parameter> coeffs_;
  Parameter xoffset_{0, 0};
  double chi2_{0};
};

void to_json(nlohmann::json& j, const CoefFunction& s);
void from_json(const nlohmann::json& j, CoefFunction& s);

using CoefFunctionPtr = std::shared_ptr<CoefFunction>;

}
