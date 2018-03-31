#include "plugin.h"

namespace DAQuiri {

void Plugin::add_definition(const SettingMeta& sm)
{
  setting_definitions_[sm.id()] = sm;
}

void Plugin::enrich(Setting& setting) const
{
  setting.enrich(setting_definitions_, true);
}

Setting Plugin::get_rich_setting(const std::string& id) const
{
  Setting set(id);
  set.enrich(setting_definitions_, true);
  return set;
}

}

