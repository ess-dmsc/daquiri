#include <core/producer_factory.h>

namespace DAQuiri {

ProducerPtr ProducerFactory::create_type(std::string type) const
{
  ProducerPtr instance;
  auto it = constructors_.find(type);
  if (it != constructors_.end())
    instance = ProducerPtr(it->second());
  return instance;
}

Setting ProducerFactory::default_settings(std::string type) const
{
  auto dummy = create_type(type);
  if (!dummy)
    return Setting();
  return dummy->settings();
}

void ProducerFactory::register_type(std::string name,
                                    std::function<Producer*(void)> constructor)
{
  if (name.empty())
    WARN("<ProducerFactory> failed to register nameless type");
  else if (constructors_.count(name))
    WARN("<ProducerFactory> type '{}' already registered", name);
  else
  {
    constructors_[name] = constructor;
    DBG("<ProducerFactory> registered '{}'", name);
  }
}

std::vector<std::string> ProducerFactory::types() const
{
  std::vector<std::string> all_types;
  for (auto& q : constructors_)
    all_types.push_back(q.first);
  return all_types;
}

void ProducerFactory::clear()
{
  constructors_.clear();
}


}

