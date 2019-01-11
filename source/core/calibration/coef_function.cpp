#include <core/calibration/coef_function.h>

namespace DAQuiri
{

CoefFunction::CoefFunction(const std::vector<double>& coeffs, double uncert, double chisq)
    : chi2_(chisq)
{
  size_t i{0};
  for (const auto& c : coeffs)
    coeffs_[i++] = Parameter(c - uncert, c, c + uncert);
}

void CoefFunction::x_offset(const Parameter& o)
{
  xoffset_ = o;
}

Parameter CoefFunction::x_offset() const
{
  return xoffset_;
}

void CoefFunction::chi2(double c2)
{
  chi2_ = c2;
}

double CoefFunction::chi2() const
{
  return chi2_;
}

std::map<int, Parameter> CoefFunction::coeffs() const
{
  return coeffs_;
}

void CoefFunction::set_coeff(int degree, const Parameter& p)
{
  coeffs_[degree] = p;
}

std::vector<double> CoefFunction::eval(const std::vector<double>& x) const
{
  std::vector<double> y;
  for (auto& q : x)
    y.push_back((*this)(q));
  return y;
}

double CoefFunction::inverse(double y, double e) const
{
  int i = 0;
  double x0 = xoffset_.value();
  double x1 = x0 + (y - (*this)(x0)) / (this->derivative(x0));
  while (i <= 100 && std::abs(x1 - x0) > e)
  {
    x0 = x1;
    x1 = x0 + (y - (*this)(x0)) / (this->derivative(x0));
    i++;
  }

  double x_adjusted = x1 - xoffset_.value();

  if (std::abs(x1 - x0) <= e)
    return x_adjusted;

  else
  {
//    WARN("<" << this->type() << "> Maximum iteration reached in CoefFunction inverse evaluation";
    return nan("");
  }
}

void to_json(nlohmann::json& j, const CoefFunction& s)
{
  j["type"] = s.type();
  for (auto c : s.coeffs())
  {
    nlohmann::json cc;
    cc["degree"] = c.first;
    cc["coefficient"] = c.second;
    j["coefficients"].push_back(cc);
  }
  j["xoffset"] = s.x_offset();
  j["chi2"] = s.chi2();
}

void from_json(const nlohmann::json& j, CoefFunction& s)
{
  if (j.count("coefficients"))
  {
    auto o = j["coefficients"];
    for (nlohmann::json::iterator it = o.begin(); it != o.end(); ++it)
    {
      int d = it.value()["degree"];
      Parameter p = it.value()["coefficient"];
      s.set_coeff(d, p);
    }
  }
  s.x_offset(j["xoffset"]);
  s.chi2(j["chi2"]);
}

}
