#pragma once

#include <core/calibration/coef_function.h>
#include <core/util/unique_mangle.h>

namespace DAQuiri
{

class CoefFunctionFactory
{
 public:
  static CoefFunctionFactory& singleton()
  {
    static CoefFunctionFactory singleton_instance;
    return singleton_instance;
  }

  CoefFunctionPtr create_type(std::string type) const;
  CoefFunctionPtr create_copy(CoefFunctionPtr other) const;
  CoefFunctionPtr create_from_json(const nlohmann::json&) const;

  void register_type(std::function<CoefFunction*(void)> constructor);
  std::vector<std::string> types() const;

  void clear();

 private:
  std::map<std::string, std::function<CoefFunction*(void)>> constructors_;

  //singleton assurance
  CoefFunctionFactory() {}
  CoefFunctionFactory(CoefFunctionFactory const&);
  void operator=(CoefFunctionFactory const&);
};

template<class T>
class CoefFunctionRegistrar
{
 public:
  CoefFunctionRegistrar()
  {
    CoefFunctionFactory::singleton().register_type([](void) -> CoefFunction* { return new T(); });
  }
};

#define DAQUIRI_REGISTER_COEF_FUNCTION(T) static DAQuiri::CoefFunctionRegistrar< T >\
  UNIQUE_MANGLE(MangledDAQuiriCoefFuncReg) ;


}
