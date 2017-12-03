#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "setting_metadata.h"
#include "color_bash.h"

namespace DAQuiri {

SettingType from_string(const std::string &type)
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
  else if (type == "precise")
    return SettingType::precise;
  else if (type == "text")
    return SettingType::text;
  else if (type == "time")
    return SettingType::time;
  else if (type == "duration")
    return SettingType::duration;
  else if (type == "menu")
    return SettingType::menu;
  else if (type == "command")
    return SettingType::command;
  else if (type == "stem")
    return SettingType::stem;
  else if (type == "indicator")
    return SettingType::indicator;
  else
    return SettingType::none;
}

std::string to_string(const SettingType &t)
{
  if (t == SettingType::boolean)
    return "boolean";
  else if (t == SettingType::integer)
    return "integer";
  else if (t == SettingType::binary)
    return "binary";
  else if (t == SettingType::pattern)
    return "pattern";
  else if (t == SettingType::floating)
    return "floating";
  else if (t == SettingType::precise)
    return "precise";
  else if (t == SettingType::text)
    return "text";
  else if (t == SettingType::time)
    return "time";
  else if (t == SettingType::duration)
    return "duration";
  else if (t == SettingType::command)
    return "command";
  else if (t == SettingType::menu)
    return "menu";
  else if (t == SettingType::indicator)
    return "indicator";
  else if (t == SettingType::stem)
    return "stem";
  else
    return "";
}

SettingMeta::SettingMeta(std::string id, SettingType type)
  : SettingMeta(id, type, id)
{}

SettingMeta::SettingMeta(std::string id, SettingType type,
                         std::string name)
  : id_(id)
  , type_ (type)
{
  contents_["name"] = name;
}

std::string SettingMeta::id() const
{
  return id_;
}

SettingType SettingMeta::type() const
{
  return type_;
}

bool SettingMeta::is(SettingType type) const
{
  return (type_ == type);
}

void SettingMeta::set_enum(int32_t idx, std::string val)
{
  enum_map_[idx] = val;
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

std::string SettingMeta::debug(std::string prepend, bool verbose) const
{
  std::string ret = col(GREEN) + to_string(type_) + col();

  if (numeric())
    ret += " " + col(CYAN) + value_range() + col();

  if (!flags_.empty())
    ret += " " + col(WHITE, NONE, DIM)
        + boost::algorithm::join(flags_, " ")
        + col();

  json cont;
  if (verbose)
    cont = contents_;
  else
    for (auto it = contents_.begin();
         it != contents_.end(); ++it)
      if ((it.key() != "min") &&
          (it.key() != "max") &&
          (it.key() != "step") &&
          (it.key() != "chans") &&
          (it.key() != "address") &&
          (it.key() != "bits"))
        cont[it.key()] = it.value();

  if (!cont.empty())
  {
    for (auto it = cont.begin();
         it != cont.end(); ++it)
      ret += " " + it.key() + "="
          + it.value().dump();
  }

  if (verbose && enum_map_.size())
  {
    ret += "\n" + prepend + " ";
    for (auto &i : enum_map_)
      ret += std::to_string(i.first) + "=\"" + i.second + "\"  ";
  }

  return ret;
}


std::string SettingMeta::value_range() const
{
  if (!numeric())
    return "";
  std::stringstream ss;
  ss << "[" << mins("m")
     << "\uFF1A" << step<double>() << "\uFF1A"
     << maxs("M") << "]";
  return ss.str();
}

std::string SettingMeta::mins(std::string def) const
{
  if (is(SettingType::integer))
    return min_str<integer_t>(def);
  else if (is(SettingType::floating))
    return min_str<floating_t>(def);
  else if (is(SettingType::precise))
    return min_str<precise_t>(def);
  return "?";
}

std::string SettingMeta::maxs(std::string def) const
{
  if (is(SettingType::integer))
    return max_str<integer_t>(def);
  else if (is(SettingType::floating))
    return max_str<floating_t>(def);
  else if (is(SettingType::precise))
    return max_str<precise_t>(def);
  return "?";
}

bool SettingMeta::numeric() const
{
  return (is(SettingType::integer)
          || is(SettingType::floating)
          || is(SettingType::precise));
}

void to_json(json& j, const SettingMeta &s)
{
  j["id"] = s.id_;
  j["type"] = to_string(s.type_);

  if (!s.contents_.empty())
    j["contents"] = s.contents_;

  if (s.is(SettingType::binary) ||
      s.is(SettingType::indicator) ||
      s.is(SettingType::menu) ||
      s.is(SettingType::stem))
    for (auto &q : s.enum_map_)
     j["items"].push_back({{"val", q.first}, {"meaning", q.second}});

  if (!s.flags_.empty())
    j["flags"] = s.flags_;
}

void from_json(const json& j, SettingMeta &s)
{
  s.id_   = j["id"];
  s.type_ = from_string(j["type"]);

  if (j.count("items"))
    for (auto it : j["items"])
      s.enum_map_[it["val"]] = it["meaning"];

  if (j.count("flags"))
    for (auto it : j["flags"])
      s.flags_.insert(it.get<std::string>());

  if (j.count("contents"))
    s.contents_ = j["contents"];
}

}


