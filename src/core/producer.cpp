#include "producer.h"
#include <fstream>

namespace DAQuiri {

bool Producer::initialize(const json& definitions)
{
  if (!definitions.empty())
    for (auto it : definitions)
      add_definition(it);

  if (setting_definitions_.empty())
  {
    DBG << "<Producer> " << this->device_name()
        << " failed to load setting definitions";
    return false;
  }

  return true;
}

json Producer::setting_definitions() const
{
  json j;
  for (auto m : setting_definitions_)
    j.push_back(m.second);
  return j;
}

void Producer::add_definition(const SettingMeta& sm)
{
  setting_definitions_[sm.id()] = sm;
}

Setting Producer::get_rich_setting(const std::string& id) const
{
  Setting set(id);
  set.enrich(setting_definitions_, true);
  return set;
}


}

