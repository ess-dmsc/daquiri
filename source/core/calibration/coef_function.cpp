#include <core/calibration/coef_function.h>

#include <sstream>
#include <iomanip>
#include <numeric>
#include <core/util/custom_logger.h>

namespace DAQuiri
{

CoefFunction::CoefFunction()
{}

CoefFunction::CoefFunction(std::vector<double> coeffs, double uncert, double rsq)
  : CoefFunction()
{
  for (size_t i=0; i < coeffs.size(); ++i)
    set_coeff(i, FitParam(coeffs[i] - uncert, coeffs[i] + uncert));
  chi2_ = rsq;
}

FitParam CoefFunction::xoffset() const
{
  return xoffset_;
}

double CoefFunction::chi2() const
{
  return chi2_;
}

size_t CoefFunction::coeff_count() const
{
  return coeffs_.size();
}

std::vector<int> CoefFunction::powers() const
{
  std::vector<int> ret;
  for (auto c : coeffs_)
    ret.push_back(c.first);
  return ret;
}

FitParam CoefFunction::get_coeff(int degree) const
{
  if (coeffs_.count(degree))
    return coeffs_.at(degree);
  else
    return FitParam();
}

std::map<int, FitParam> CoefFunction::get_coeffs() const
{
  return coeffs_;
}

void CoefFunction::set_xoffset(const FitParam& o)
{
  xoffset_ = o;
}

void CoefFunction::set_chi2(double c2)
{
  chi2_ = c2;
}

void CoefFunction::set_coeff(int degree, const FitParam& p)
{
  coeffs_[degree] = p;
}

void CoefFunction::set_coeff(int degree, const UncertainDouble& p)
{
  coeffs_[degree].set_value(p);
  //bounds?
}

std::vector<double> CoefFunction::eval_array(const std::vector<double> &x) const
{
  std::vector<double> y;
  for (auto &q : x)
    y.push_back(this->eval(q));
  return y;
}

double CoefFunction::eval_inverse(double y, double e) const
{
  int i=0;
  double x0 = xoffset_.value().value();
  double x1 = x0 + (y - this->eval(x0)) / (this->derivative(x0));
  while( i<=100 && std::abs(x1-x0) > e)
  {
    x0 = x1;
    x1 = x0 + (y - this->eval(x0)) / (this->derivative(x0));
    i++;
  }

  double x_adjusted = x1 - xoffset_.value().value();

  if(std::abs(x1-x0) <= e)
    return x_adjusted;

  else
  {
//    WARN("<" << this->type() << "> Maximum iteration reached in CoefFunction inverse evaluation";
    return nan("");
  }
}

std::vector<double> CoefFunction::coeffs_consecutive() const
{
  std::vector<double> ret;
  int top = 0;
  for (auto &c : coeffs_)
    if (c.first > top)
      top = c.first;
  ret.resize(top+1, 0);
  for (auto &c : coeffs_)
    ret[c.first] = c.second.value().value();
  return ret;
}

void to_json(json& j, const CoefFunction& s)
{
  for (auto c : s.get_coeffs())
  {
    json cc;
    cc["degree"] = c.first;
    cc["coefficient"] = c.second;
    j["coefficients"].push_back(cc);
  }
  j["xoffset"] = s.xoffset();
  j["chi2"] = s.chi2();
}

void from_json(const json& j, CoefFunction& s)
{
  if (j.count("coefficients"))
  {
    auto o = j["coefficients"];
    for (json::iterator it = o.begin(); it != o.end(); ++it)
    {
      int d = it.value()["degree"];
      FitParam p = it.value()["coefficient"];
      s.set_coeff(d, p);
    }
  }
  s.set_xoffset(j["xoffset"]);
  s.set_chi2(j["chi2"]);
}

CoefFuncPtr CoefFuncFactory::create_type(std::string type) const
{
  CoefFuncPtr instance;
  auto it = constructors.find(type);
  if (it != constructors.end())
    instance = CoefFuncPtr(it->second());
  if (instance.operator bool())
    return instance;
  return CoefFuncPtr();
}

void CoefFuncFactory::register_type(std::string name,
                                     std::function<CoefFunction*(void)> typeConstructor)
{
  INFO("<CoefFuncFactory> registering CoefFunction '{}'", name);
  constructors[name] = typeConstructor;
}

std::vector<std::string> CoefFuncFactory::types() const
{
  std::vector<std::string> all_types;
  for (auto &q : constructors)
    all_types.push_back(q.first);
  return all_types;
}

}
