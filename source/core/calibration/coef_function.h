#pragma once

#include <vector>
#include <map>
#include <core/calibration/parameter.h>

namespace DAQuiri
{

class CoefFunction
{
public:
  CoefFunction();
  CoefFunction(std::vector<double> coeffs, double uncert, double rsq);

  FitParam xoffset() const;
  double chi2() const;
  size_t coeff_count() const;
  std::vector<int> powers() const;
  FitParam get_coeff(int degree) const;
  std::map<int, FitParam> get_coeffs() const;
  std::vector<double> coeffs_consecutive() const;

  void set_xoffset(const FitParam& o);
  void set_chi2(double);
  void set_coeff(int degree, const FitParam& p);
  void set_coeff(int degree, const UncertainDouble& p);

  double eval_inverse(double y, double e = 0.2) const;
  std::vector<double> eval_array(const std::vector<double> &x) const;

  //TO IMPLEMENT IN CHILDREN
  virtual std::string type() const = 0;
  virtual std::string to_string() const = 0;
  virtual std::string to_UTF8() const = 0;
  virtual std::string to_markup() const = 0;

  virtual double eval(double x) const = 0;
  virtual double derivative(double x) const = 0;

protected:
  std::map<int, FitParam> coeffs_;
  FitParam xoffset_ {0};
  double chi2_ {0};
};

void to_json(json& j, const CoefFunction &s);
void from_json(const json& j, CoefFunction &s);


using CoefFuncPtr = std::shared_ptr<CoefFunction>;

class CoefFuncFactory {
public:
  static CoefFuncFactory& singleton()
  {
    static CoefFuncFactory singleton_instance;
    return singleton_instance;
  }

  CoefFuncPtr create_type(std::string type) const;
  void register_type(std::string name, std::function<CoefFunction*(void)> typeConstructor);
  std::vector<std::string> types() const;

private:
  std::map<std::string, std::function<CoefFunction*(void)>> constructors;

  //singleton assurance
  CoefFuncFactory() {}
  CoefFuncFactory(CoefFuncFactory const&);
  void operator=(CoefFuncFactory const&);
};

template<class T>
class CoefFuncRegistrar {
public:
  CoefFuncRegistrar(std::string name)
  {
    CoefFuncFactory::singleton().register_type(name,
                                               [](void) -> CoefFunction * { return new T();});
  }
};

}
