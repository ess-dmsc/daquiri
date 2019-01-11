#pragma once

#include <core/calibration/coef_function.h>

namespace DAQuiri
{

class Polynomial : public CoefFunction
{
 public:
  using CoefFunction::CoefFunction;

  std::string type() const override { return "Polynomial"; }
  Polynomial* clone() const override { return new Polynomial(*this); }
  double operator() (double x) const override;
  double derivative(double) const override;

  std::string debug() const override;
  std::string to_UTF8(int precision, bool with_rsq) const override;
  std::string to_markup(int precision, bool with_rsq) const override;
};

}