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

SettingMeta::SettingMeta(std::string id, SettingType type)
  : SettingMeta(id, type, id)
{}

SettingMeta::SettingMeta(std::string id, SettingType type, std::string name)
  : id_(id)
  , type_ (type)
{
  contents_["name"] = name;
}

bool SettingMeta::operator!= (const SettingMeta& other) const
{
  return !operator==(other);
}

bool SettingMeta::operator== (const SettingMeta& other) const
{
  if (id_ != other.id_) return false;
  if (contents_ != other.contents_) return false;
  if (enum_map_ != other.enum_map_) return false;
  if (flags_ != other.flags_) return false;
  return true;
}

std::string SettingMeta::id() const
{
  return id_;
}

SettingType SettingMeta::type() const
{
  return type_;
}

std::string SettingMeta::enum_name(int32_t idx) const
{
  if (enum_map_.count(idx))
    return enum_map_.at(idx);
  return "";
}

std::list<std::string> SettingMeta::enum_names() const
{
  std::list<std::string> ret;
  for (auto i : enum_map_)
    ret.push_back(i.second);
  return ret;
}

std::string SettingMeta::get_string(std::string name, std::string default_val) const
{
  if (contents_.count(name) && contents_.at(name).is_string())
    return contents_[name];
  return default_val;
}

void SettingMeta::set_flag(std::string f)
{
  flags_.insert(f);
}

void SettingMeta::remove_flag(std::string f)
{
  if (flags_.count(f))
    flags_.erase(f);
}

void SettingMeta::set_flags(std::initializer_list<std::string> fs)
{
  for (auto f : fs)
    flags_.insert(f);
}

bool SettingMeta::has_flag(std::string f)
{
  return flags_.count(f);
}

SettingMeta SettingMeta::stripped() const
{
  SettingMeta s;
  s.id_ = id_;
  s.type_ = type_;
  return s;
}

bool SettingMeta::meaningful() const
{
  return (!flags_.empty() ||
          !contents_.empty() ||
          !enum_map_.empty());
}

std::string SettingMeta::debug(std::string prepend) const
{
  std::string ret = to_string(type_);
  if (numeric())
    ret += value_range();

  if (!flags_.empty())
  {
    std::string flgs;
    for (auto &q : flags_)
      flgs += q + " ";
    boost::algorithm::trim_if(flgs, boost::algorithm::is_any_of("\r\n\t "));
    if (!flgs.empty())
      ret += " flags=\"" + flgs + "\"";
  }

  if (enum_map_.size())
  {
    ret += "\n" + prepend + " ";
    for (auto &i : enum_map_)
      ret += std::to_string(i.first) + "=\"" + i.second + "\"  ";
  }
  if (!contents_.empty())
    ret += "contents_=" + contents_.dump();

  return ret;
}


std::string SettingMeta::value_range() const
{
  if (numeric())
  {
    std::stringstream ss;
    ss << "[" << min<double>() << " \uFF1A "
       << step<double>() << " \uFF1A "
       << max<double>() << "]";
    return ss.str();
  }
  return "";
}

bool SettingMeta::numeric() const
{
  return ((type_ == SettingType::integer)
          || (type_ == SettingType::floating)
          || (type_ == SettingType::floating_precise));
}

void to_json(json& j, const SettingMeta &s)
{
  j["id"] = s.id_;
  j["type"] = to_string(s.type_);

  if (!s.contents_.empty())
    j["contents_"] = s.contents_;

  if ((s.type_ == SettingType::binary) ||
      (s.type_ == SettingType::indicator) ||
      (s.type_ == SettingType::int_menu) ||
      (s.type_ == SettingType::stem))
    for (auto &q : s.enum_map_)
     j["items"].push_back({{"val", q.first}, {"meaning", q.second}});

  if (!s.flags_.empty())
    j["flags"] = s.flags_;
}

void from_json(const json& j, SettingMeta &s)
{
  s.id_          = j["id"];
  s.type_ = to_type(j["type"]);

  if (j.count("items"))
    for (auto it : j["items"])
      s.enum_map_[it["val"]] = it["meaning"];

  if (j.count("flags"))
    for (auto it : j["flags"])
      s.flags_.insert(it.get<std::string>());
}

}


