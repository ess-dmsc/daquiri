#include "producer.h"
#include <fstream>

namespace DAQuiri {

bool Producer::load_setting_definitions(const std::string &file)
{
  std::ifstream ifile (file);
  json j;
  if (ifile.is_open())
    ifile >> j;

  settings_from_json(j);

  if (!setting_definitions_.empty())
  {
    DBG << "<Producer> " << this->device_name()
        << " retrieved " << setting_definitions_.size()
        << " setting definitions";
    profile_path_ = file;
    return true;
  }
  else
  {
    DBG << "<Producer> " << this->device_name()
        << " failed to load setting definitions";
    profile_path_.clear();
    return false;
  }
}

void Producer::settings_from_json(const json& j)
{
  for (auto it : j)
  {
    SettingMeta m = it;
    setting_definitions_[m.id()] = m;
  }
}

json Producer::settings_to_json() const
{
  json j;
  for (auto m : setting_definitions_)
    j.push_back(m.second);
  return j;
}

void Producer::save_setting_definitions(const std::string &file)
{
  std::ofstream ofile (file);
  if (ofile.is_open())
    ofile << settings_to_json();
}

Setting Producer::get_rich_setting(const std::string& id) const
{
  Setting set(id);
  set.enrich(setting_definitions_, true);
  return set;
}


}

