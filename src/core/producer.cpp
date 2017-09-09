#include "producer.h"
#include <fstream>

namespace DAQuiri {

ProducerStatus operator|(ProducerStatus a, ProducerStatus b)
{
  return static_cast<ProducerStatus>
      (static_cast<int>(a) | static_cast<int>(b));
}

ProducerStatus operator&(ProducerStatus a, ProducerStatus b)
{
  return static_cast<ProducerStatus>
      (static_cast<int>(a) & static_cast<int>(b));
}

ProducerStatus operator^(ProducerStatus a, ProducerStatus b)
{
  return static_cast<ProducerStatus>
      (static_cast<int>(a) ^ static_cast<int>(b));
}

void Producer::initialize(const json& definitions)
{
  if (!definitions.empty())
    for (auto it : definitions)
      add_definition(it);

  if (setting_definitions_.empty())
  {
    DBG << "<Producer> " << this->plugin_name()
        << " failed to load setting definitions";
  }
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

Setting Producer::enrich_and_toggle_presets(Setting set) const
{
  set.enrich(setting_definitions_, true);
  set.enable_if_flag(!(status_ & booted), "preset");
  return set;
}



}

