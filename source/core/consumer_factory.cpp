#include <core/consumer_factory.h>
#include <core/util/logger.h>

namespace DAQuiri {

ConsumerPtr ConsumerFactory::create_type(std::string type) const
{
  auto it = constructors_.find(type);
  if(it != constructors_.end())
    return ConsumerPtr(it->second());
  else
    return ConsumerPtr();
}

ConsumerPtr ConsumerFactory::create_copy(ConsumerPtr other) const
{
  if (other)
    return ConsumerPtr(other->clone());
  return ConsumerPtr();
}

ConsumerPtr ConsumerFactory::create_from_prototype(const ConsumerMetadata& tem) const
{
  ConsumerPtr instance = create_type(tem.type());
  if (instance)
  {
    instance->from_prototype(tem);
    return instance;
  }
  return ConsumerPtr();
}

ConsumerPtr ConsumerFactory::create_from_h5(hdf5::node::Group &group, bool withdata) const
{
  if (!group.attributes.exists("type"))
    return ConsumerPtr();

  std::string type;
  group.attributes["type"].read(type);
  ConsumerPtr instance = create_type(type);
  if (instance)
  {
    instance->load(group, withdata);
    return instance;
  }

  return ConsumerPtr();
}

ConsumerMetadata ConsumerFactory::create_prototype(std::string type) const
{
  auto it = prototypes_.find(type);
  if(it != prototypes_.end())
    return it->second;
  else
    return ConsumerMetadata();
}

void ConsumerFactory::register_type(ConsumerMetadata tt,
                                    std::function<Consumer*(void)> constructor)
{
  auto name = tt.type();
  if (name.empty())
    WARN("<ConsumerFactory> failed to register nameless type");
  else if (constructors_.count(tt.type()))
    WARN("<ConsumerFactory> type '{}' already registered", tt.type());
  else
  {
    constructors_[tt.type()] = constructor;
    prototypes_[tt.type()] = tt;
    DBG("<ConsumerFactory> registered '{}'", tt.type());
  }
}

std::vector<std::string> ConsumerFactory::types() const
{
  std::vector<std::string> all_types;
  for (auto &q : constructors_)
    all_types.push_back(q.first);
  return all_types;
}

void ConsumerFactory::clear()
{
  constructors_.clear();
  prototypes_.clear();
}


}
