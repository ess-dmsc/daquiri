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

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "setting_metadata.h"
#include "util.h"

namespace DAQuiri {

SettingType to_type(const std::string &type)
{
  if (type == "boolean")
    return SettingType::boolean;
  else if (type == "integer")
    return SettingType::integer;
  else if (type == "binary")
    return SettingType::binary;
  else if (type == "pattern")
    return SettingType::pattern;
  else if (type == "floating")
    return SettingType::floating;
  else if (type == "floating_precise")
    return SettingType::floating_precise;
  else if (type == "text")
    return SettingType::text;
  else if (type == "color")
    return SettingType::color;
  else if (type == "time")
    return SettingType::time;
  else if (type == "time_duration")
    return SettingType::time_duration;
  else if (type == "detector")
    return SettingType::detector;
  else if (type == "file_path")
    return SettingType::file_path;
  else if (type == "dir_path")
    return SettingType::dir_path;
  else if (type == "int_menu")
    return SettingType::int_menu;
  else if (type == "command")
    return SettingType::command;
  else if (type == "stem")
    return SettingType::stem;
  else if (type == "indicator")
    return SettingType::indicator;
  else
    return SettingType::none;
}

std::string to_string(SettingType type)
{
  if (type == SettingType::boolean)
    return "boolean";
  else if (type == SettingType::integer)
    return "integer";
  else if (type == SettingType::binary)
    return "binary";
  else if (type == SettingType::pattern)
    return "pattern";
  else if (type == SettingType::floating)
    return "floating";
  else if (type == SettingType::floating_precise)
    return "floating_precise";
  else if (type == SettingType::text)
    return "text";
  else if (type == SettingType::color)
    return "color";
  else if (type == SettingType::detector)
    return "detector";
  else if (type == SettingType::time)
    return "time";
  else if (type == SettingType::time_duration)
    return "time_duration";
  else if (type == SettingType::file_path)
    return "file_path";
  else if (type == SettingType::dir_path)
    return "dir_path";
  else if (type == SettingType::command)
    return "command";
  else if (type == SettingType::int_menu)
    return "int_menu";
  else if (type == SettingType::indicator)
    return "indicator";
  else if (type == SettingType::stem)
    return "stem";
  else
    return "";
}


bool SettingMeta::shallow_equals(const SettingMeta& other) const
{
  return (id_ == other.id_);
}

bool SettingMeta::operator!= (const SettingMeta& other) const
{
  return !operator==(other);
}

bool SettingMeta::operator== (const SettingMeta& other) const
{
  if (id_ != other.id_) return false;
  if (name != other.name) return false;
  if (unit != other.unit) return false;
  if (minimum != other.minimum) return false;
  if (maximum != other.maximum) return false;
  if (step != other.step) return false;
  if (writable != other.writable) return false;
  if (description != other.description) return false;
  if (address != other.address) return false;
  if (int_menu_items != other.int_menu_items) return false;
  if (flags != other.flags) return false;
  return true;
}

SettingMeta SettingMeta::stripped() const
{
  SettingMeta s;
  s.id_ = id_;
  s.setting_type = setting_type;
  return s;
}

bool SettingMeta::meaningful() const
{
  return (operator!=(this->stripped()));
}

std::string SettingMeta::debug(std::string prepend) const
{
  std::string ret = to_string(setting_type);
  if (!name.empty())
    ret += "(" + name + ")";
  if (address != -1)
    ret += " " + std::to_string(address);
  if (writable)
    ret += " wr";
  if ((setting_type == SettingType::command) && !visible)
    ret += " invis";
  if ((setting_type == SettingType::stem) && saveworthy)
    ret += " savew";
  if ((setting_type == SettingType::binary) || (setting_type == SettingType::pattern))
    ret += " wsize=" + std::to_string(maximum);
  if (is_numeric())
  {
    std::stringstream ss;
    ss << " [" << minimum << "\uFF1A" << step << "\uFF1A" << maximum << "]";
    ret += ss.str();
  }
  if (!unit.empty())
    ret += " unit=" + unit;
  if (!description.empty())
    ret += " \"" + description + "\"";

  if (!flags.empty()) {
    std::string flgs;
    for (auto &q : flags)
      flgs += q + " ";
    boost::algorithm::trim_if(flgs, boost::algorithm::is_any_of("\r\n\t "));
    if (!flgs.empty())
      ret += " flags=\"" + flgs + "\"";
  }

  if (int_menu_items.size())
  {
    ret += "\n" + prepend + " ";
    for (auto &i : int_menu_items)
      ret += std::to_string(i.first) + "=\"" + i.second + "\"  ";
  }

  return ret;
}


std::string SettingMeta::value_range() const
{
  if (is_numeric())
  {
    std::stringstream ss;
    ss << "[" << minimum << " \uFF1A " << step << " \uFF1A " << maximum << "]";
    return ss.str();
  }
  else if ((setting_type == SettingType::binary) || (setting_type == SettingType::pattern))
  {
    std::stringstream ss;
    ss << to_string(setting_type) << maximum;
    return ss.str();
  }
  else
    return to_string(setting_type);
}

bool SettingMeta::is_numeric() const
{
  return ((setting_type == SettingType::integer)
          || (setting_type == SettingType::floating)
          || (setting_type == SettingType::floating_precise));
}

void to_json(json& j, const SettingMeta &s)
{
  j["id"] = s.id_;
  j["type"] = to_string(s.setting_type);
  j["address"] = s.address;
  j["max_indices"] = s.max_indices;
  j["writable"] = s.writable;

  if (!s.name.empty())
    j["name"] = s.name;
  if (!s.unit.empty())
    j["unit"] = s.unit;
  if (!s.description.empty())
    j["description"] = s.description;

  if (s.setting_type == SettingType::command)
    j["visible"] = s.visible;
  if (s.setting_type == SettingType::stem)
    j["saveworthy"] = s.saveworthy;

  if (s.is_numeric())
  {
    j["step"] = s.step;
    j["minimum"] = s.minimum;
    j["maximum"] = s.maximum;
  }
  else if ((s.setting_type == SettingType::binary) ||
           (s.setting_type == SettingType::pattern))
    j["word_size"] = s.maximum;

  if ((s.setting_type == SettingType::binary) ||
      (s.setting_type == SettingType::indicator) ||
      (s.setting_type == SettingType::int_menu) ||
      (s.setting_type == SettingType::stem))
    for (auto &q : s.int_menu_items)
     j["items"].push_back({{"val", q.first}, {"meaning", q.second}});

  if (!s.flags.empty())
    j["flags"] = s.flags;
}

void from_json(const json& j, SettingMeta &s)
{
  s.id_          = j["id"];
  s.setting_type = to_type(j["type"]);
  s.address      = j["address"];
  s.max_indices  = j["max_indices"];
  s.writable     = j["writable"];

  if (j.count("name"))
    s.name = j["name"];
  if (j.count("unit"))
    s.unit = j["unit"];
  if (j.count("description"))
    s.description = j["description"];

  if (j.count("visible"))
    s.visible = j["visible"];
  if (j.count("saveworthy"))
    s.saveworthy = j["saveworthy"];

  if (s.is_numeric())
  {
    s.step = j["step"];
    s.minimum = j["minimum"];
    s.maximum = j["maximum"];
  }
  else if (j.count("word_size"))
    s.maximum = j["word_size"];

  if (j.count("items"))
    for (auto it : j["items"])
      s.int_menu_items[it["val"]] = it["meaning"];

  if (j.count("flags"))
    for (auto it : j["flags"])
      s.flags.insert(it.get<std::string>());
}

}


