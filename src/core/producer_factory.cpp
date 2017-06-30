#include "producer_factory.h"

namespace DAQuiri {

ProducerPtr ProducerFactory::create_type(std::string type, std::string file)
{
  ProducerPtr instance;
  auto it = constructors.find(type);
  if (it != constructors.end())
    instance = ProducerPtr(it->second());
  if (instance.operator bool() && instance->load_setting_definitions(file))
    return instance;
  return ProducerPtr();
}

ProducerPtr ProducerFactory::create_json(std::string type, std::string file)
{
  ProducerPtr instance;
  auto it = constructors.find(type);
  if (it != constructors.end())
    instance = ProducerPtr(it->second());
  if (instance.operator bool() && instance->load_setting_json(file))
    return instance;
  return ProducerPtr();
}

void ProducerFactory::register_type(std::string name, std::function<Producer*(void)> typeConstructor)
{
  LINFO << "<ProducerFactory> registering source '" << name << "'";
  constructors[name] = typeConstructor;
}

const std::vector<std::string> ProducerFactory::types() {
  std::vector<std::string> all_types;
  for (auto &q : constructors)
    all_types.push_back(q.first);
  return all_types;
}

}

