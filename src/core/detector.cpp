/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 ******************************************************************************/

#include "detector.h"
#include "util.h"

namespace DAQuiri {

Detector::Detector()
{
  settings_.metadata.setting_type = SettingType::stem;
}

Detector::Detector(std::string name)
  : Detector()
{
  name_ = name;
}

std::string Detector::name() const
{
  return name_;
}

std::string Detector::type() const
{
  return type_;
}

std::list<Setting> Detector::optimizations() const
{
  return settings_.branches.my_data_;
}

void Detector::set_name(const std::string& n)
{
  name_ = n;
}

void Detector::set_type(const std::string& t)
{
  type_ = t;
}

void Detector::add_optimizations(const std::list<Setting>& l, bool writable_only)
{
  for (auto s : l)
  {
    if (writable_only && !s.metadata.writable)
      continue;
    s.indices.clear();
    settings_.branches.replace(s);
  }
}

Setting Detector::get_setting(std::string id) const
{
  return settings_.get_setting(Setting(id), Match::id);
}

void Detector::clear_optimizations()
{
  settings_ = Setting("Optimizations");
  settings_.metadata.setting_type = SettingType::stem;
}


bool Detector::operator== (const Detector& other) const
{
  return ((name_ == other.name_) &&
          (type_ == other.type_) &&
          (settings_ == other.settings_));
}

bool Detector::operator!= (const Detector& other) const
{
  return !operator==(other);
}

std::string Detector::debug(std::string prepend) const
{
  std::stringstream ss;
  ss << name_ << "(" << type_ << ")\n";
  ss << prepend << k_branch_end << settings_.debug(prepend + "  ");
  return ss.str();
}

void to_json(json& j, const Detector &s)
{
  j = s.to_json(true);
}

json Detector::to_json(bool options) const
{
  json j;
  j["name"] = name_;
  j["type"] = type_;
  if (options && !settings_.branches.empty())
    j["optimizations"] = settings_;
  return j;
}

void from_json(const json& j, Detector &s)
{
  s.name_ = j["name"];
  s.type_ = j["type"];
  if (j.count("optimizations"))
    s.settings_ = j["optimizations"];
}

}
