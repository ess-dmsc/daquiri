#include "producer.h"
#include <fstream>

namespace DAQuiri {

bool Producer::load_setting_definitions(const std::__cxx11::string &file)
{
//  pugi::xml_document doc;

//  if (!doc.load_file(file.c_str()))
//    return false;

//  pugi::xml_node root = doc.first_child();
//  if (!root)
//    return false;

//  for (pugi::xml_node node : root.children())
//  {
//    if (node.name() &&
//        (std::string(node.name()) == SettingMeta().xml_element_name()))
//    {
//      SettingMeta newset(node);
//      if (newset != SettingMeta())
//        setting_definitions_[newset.id_] = newset;
//    }
//  }

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

bool Producer::load_setting_json(const std::string& file)
{
  std::ifstream ifile (file);
  json j;
  if (ifile.is_open())
    ifile >> j;

  for (auto it : j)
  {
    SettingMeta m = it;
    setting_definitions_[m.id()] = m;
  }

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

bool Producer::save_setting_json(const std::string& file)
{
  json j;
  for (auto m : setting_definitions_)
    j.push_back(m.second);
  std::ofstream ofile (file);
  if (ofile.is_open())
    ofile << j;
}

bool Producer::save_setting_definitions(const std::__cxx11::string &file)
{
//  pugi::xml_document doc;
//  pugi::xml_node root = doc.append_child();
//  root.set_name("SettingDefinitions");
//  for (auto &q : setting_definitions_)
//    q.second.to_xml(root);

//  return doc.save_file(file.c_str());
}

Setting Producer::get_rich_setting(const std::string& id) const
{
  Setting set(id);
  set.enrich(setting_definitions_, true);
  return set;
}


}

