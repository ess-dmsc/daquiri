#include "consumer_factory.h"
#include "custom_logger.h"

namespace DAQuiri {

ConsumerPtr ConsumerFactory::create_type(std::string type)
{
  auto it = constructors.find(type);
  if(it != constructors.end())
    return ConsumerPtr(it->second());
  else
    return ConsumerPtr();
}

ConsumerPtr ConsumerFactory::create_copy(ConsumerPtr other)
{
  return ConsumerPtr(other->clone());
}

ConsumerPtr ConsumerFactory::create_from_prototype(const ConsumerMetadata& tem)
{
//  DBG << "<ConsumerFactory> creating " << tem.type();
  ConsumerPtr instance = create_type(tem.type());
  if (instance && instance->from_prototype(tem))
    return instance;
  return ConsumerPtr();
}

ConsumerPtr ConsumerFactory::create_from_h5(H5CC::Group &group, bool withdata)
{
  if (!group.has_attribute("type"))
    return ConsumerPtr();

//  DBG << "<ConsumerFactory> making " << root.attribute("type").value();

  ConsumerPtr instance = create_type(group.read_attribute<std::string>("type"));
  if (instance && instance->load(group, withdata))
    return instance;

  return ConsumerPtr();
}

ConsumerMetadata ConsumerFactory::create_prototype(std::string type)
{
  auto it = prototypes.find(type);
  if(it != prototypes.end())
    return it->second;
  else
    return ConsumerMetadata();
}

void ConsumerFactory::register_type(ConsumerMetadata tt, std::function<Consumer*(void)> typeConstructor)
{
  constructors[tt.type()] = typeConstructor;
  prototypes[tt.type()] = tt;
  INFO << "<ConsumerFactory> registered '" << tt.type() << "'";
}

const std::vector<std::string> ConsumerFactory::types()
{
  std::vector<std::string> all_types;
  for (auto &q : constructors)
    all_types.push_back(q.first);
  return all_types;
}

}
