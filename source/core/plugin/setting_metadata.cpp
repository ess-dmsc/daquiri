#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <core/plugin/setting_metadata.h>
#include <core/util/color_bash.h>

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
  values_["name"] = name;
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

bool SettingMeta::is_numeric() const
{
  return (is(SettingType::integer)
      || is(SettingType::floating)
      || is(SettingType::precise));
}

void SettingMeta::set_enum(int32_t idx, std::string val)
{
  enum_map_[idx] = val;
}

void SettingMeta::set_enums(int32_t start_idx, std::list<std::string> vals)
{
  for (const auto& v : vals)
    set_enum(start_idx++, v);
}

bool SettingMeta::has_enum(int32_t idx) const
{
  return (enum_map_.count(idx) != 0);
}

std::string SettingMeta::enum_name(int32_t idx) const
{
  if (has_enum(idx))
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

std::map<int32_t, std::string> SettingMeta::enum_map() const
{
  return enum_map_;
}

std::string SettingMeta::get_string(std::string name, std::string default_val) const
{
  if (values_.count(name) && values_.at(name).is_string())
    return values_[name];
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
          !values_.empty() ||
          !enum_map_.empty());
}

std::string SettingMeta::debug(std::string prepend, bool verbose) const
{
  std::string ret = col(GREEN) + to_string(type_) + col();

  if (is_numeric())
    ret += " " + col(CYAN) + value_range() + col();

  if (!flags_.empty())
    ret += " " + col(WHITE, NONE, DIM)
        + boost::algorithm::join(flags_, " ")
        + col();

  json cont;
  if (verbose)
    cont = values_;
  else
    for (auto it = values_.begin();
         it != values_.end(); ++it)
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
  if (!is_numeric())
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

// \todo change names and convert existing files
void to_json(json& j, const SettingMeta &s)
{
  j["id"] = s.id_;
  j["type"] = to_string(s.type_);

  if (!s.values_.empty())
    j["contents"] = s.values_;

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
    s.values_ = j["contents"];
}

}


