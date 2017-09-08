#include "producer_factory.h"

namespace DAQuiri {

ProducerPtr ProducerFactory::create_type(std::string type) const
{
  ProducerPtr instance;
  auto it = constructors_.find(type);
  if (it != constructors_.end())
    instance = ProducerPtr(it->second());
  if (!instance)
    return ProducerPtr();
  instance->initialize(json());
  return instance;
}

Setting ProducerFactory::default_settings(std::string type) const
{
  auto dummy = create_type(type);
  if (!dummy)
    return Setting();
  Setting settings({dummy->plugin_name(), SettingType::stem});
  dummy->write_settings_bulk(settings);
  dummy->read_settings_bulk(settings);
  return settings;
}

void ProducerFactory::register_type(std::string name,
                                    std::function<Producer*(void)> constructor)
{
  if (name.empty())
    INFO << "<ProducerFactory> attempting to register nameless type";
  else if (constructors_.count(name))
    INFO << "<ProducerFactory> type '" << name << "' already registered";
  else
  {
    constructors_[name] = constructor;
    INFO << "<ProducerFactory> registered '" << name << "'";
  }
}

std::vector<std::string> ProducerFactory::types() const
{
  std::vector<std::string> all_types;
  for (auto &q : constructors_)
    all_types.push_back(q.first);
  return all_types;
}

}

