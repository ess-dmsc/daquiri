#include "consumer_factory.h"
#include "custom_logger.h"

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
  return ConsumerPtr(other->clone());
}

ConsumerPtr ConsumerFactory::create_from_prototype(const ConsumerMetadata& tem) const
{
//  DBG << "<ConsumerFactory> creating " << tem.type();
  ConsumerPtr instance = create_type(tem.type());
  if (instance && instance->from_prototype(tem))
    return instance;
  return ConsumerPtr();
}

#ifdef DAQUIRI_USE_H5
ConsumerPtr ConsumerFactory::create_from_h5(hdf5::node::Group &group, bool withdata) const
{
  if (!group.attributes.exists("type"))
    return ConsumerPtr();

//  DBG << "<ConsumerFactory> making " << root.attribute("type").value();

  std::string type;
  group.attributes["type"].read(type);
//  auto type = group.read_attribute<std::string>("type");
  ConsumerPtr instance = create_type(type);
  if (instance && instance->load(group, withdata))
    return instance;

  return ConsumerPtr();
}
#endif

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
    INFO << "<ConsumerFactory> attempting to register nameless type";
  else if (constructors_.count(tt.type()))
    INFO << "<ConsumerFactory> type '" << tt.type() << "' already registered";
  else
  {
    constructors_[tt.type()] = constructor;
    prototypes_[tt.type()] = tt;
    INFO << "<ConsumerFactory> registered '" << tt.type() << "'";
  }
}

std::vector<std::string> ConsumerFactory::types() const
{
  std::vector<std::string> all_types;
  for (auto &q : constructors_)
    all_types.push_back(q.first);
  return all_types;
}

}
