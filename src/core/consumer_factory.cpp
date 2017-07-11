#include "consumer_factory.h"
#include "custom_logger.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace DAQuiri {

SinkPtr ConsumerFactory::create_type(std::string type)
{
  auto it = constructors.find(type);
  if(it != constructors.end())
    return SinkPtr(it->second());
  else
    return SinkPtr();
}

SinkPtr ConsumerFactory::create_copy(SinkPtr other)
{
  return SinkPtr(other->clone());
}

SinkPtr ConsumerFactory::create_from_prototype(const ConsumerMetadata& tem)
{
//  DBG << "<ConsumerFactory> creating " << tem.type();
  SinkPtr instance = create_type(tem.type());
  if (instance && instance->from_prototype(tem))
    return instance;
  return SinkPtr();
}

SinkPtr ConsumerFactory::create_from_h5(H5CC::Group &group, bool withdata)
{
  if (!group.has_attribute("type"))
    return SinkPtr();

//  DBG << "<ConsumerFactory> making " << root.attribute("type").value();

  SinkPtr instance = create_type(group.read_attribute<std::string>("type"));
  if (instance && instance->load(group, withdata))
    return instance;

  return SinkPtr();
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
  LINFO << "<ConsumerFactory> registering sink type '" << tt.type() << "'";
  constructors[tt.type()] = typeConstructor;
  prototypes[tt.type()] = tt;
}

const std::vector<std::string> ConsumerFactory::types()
{
  std::vector<std::string> all_types;
  for (auto &q : constructors)
    all_types.push_back(q.first);
  return all_types;
}

}
