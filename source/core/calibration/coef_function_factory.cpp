#include <core/calibration/coef_function_factory.h>

#include <core/util/logger.h>

namespace DAQuiri
{

void CoefFunctionFactory::register_type(std::function<CoefFunction*(void)> constructor)
{
  auto name = CoefFunctionPtr(constructor())->type();
  if (name.empty())
    WARN("<CoefFunctionFactory> failed to register nameless type");
  else if (constructors_.count(name))
    WARN("<CoefFunctionFactory> type '{}' already registered", name);
  else
  {
    constructors_[name] = constructor;
    DBG("<CoefFunctionFactory> registered '{}'", name);
  }
}

CoefFunctionPtr CoefFunctionFactory::create_type(std::string type) const
{
  auto it = constructors_.find(type);
  if (it != constructors_.end())
    return CoefFunctionPtr(it->second());
  else
    return CoefFunctionPtr();
}

CoefFunctionPtr CoefFunctionFactory::create_copy(CoefFunctionPtr other) const
{
  if (other)
    return CoefFunctionPtr(other->clone());
  return CoefFunctionPtr();
}

CoefFunctionPtr CoefFunctionFactory::create_from_json(const nlohmann::json& j) const
{
  std::string type = j["type"];
  auto ret = create_type(type);
  from_json(j, *ret);
  return ret;
}

std::vector<std::string> CoefFunctionFactory::types() const
{
  std::vector<std::string> all_types;
  for (auto& q : constructors_)
    all_types.push_back(q.first);
  return all_types;
}

void CoefFunctionFactory::clear()
{
  constructors_.clear();
}


}
