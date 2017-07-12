#include "producer_factory.h"

namespace DAQuiri {

ProducerPtr ProducerFactory::create_type(std::string type, const json &profile)
{
  ProducerPtr instance;
  auto it = constructors.find(type);
  if (it != constructors.end())
    instance = ProducerPtr(it->second());
  if (instance.operator bool() &&
      instance->initialize(profile))
    return instance;
  return ProducerPtr();
}

void ProducerFactory::register_type(std::string name,
                                    std::function<Producer*(void)> typeConstructor)
{
  constructors[name] = typeConstructor;
  LINFO << "<ProducerFactory> registered '" << name << "'";
}

const std::vector<std::string> ProducerFactory::types()
{
  std::vector<std::string> all_types;
  for (auto &q : constructors)
    all_types.push_back(q.first);
  return all_types;
}

}

