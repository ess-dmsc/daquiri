#pragma once

#include <string>
#include <map>
#include <set>
#include <list>
#include "precise_float.h"

#include "json.hpp"
using namespace nlohmann;

#define TT template<typename T>

namespace DAQuiri {

using integer_t = int64_t;
using floating_t = double;
using precise_t = PreciseFloat;

enum class SettingType {none,
                        stem,       // as branches
                        time,       // as ptime
                        duration,   // as time_duration
                        pattern,    // as Pattern

                        floating,   // as double
                        precise,    // as PreciseFloat

                        boolean,    // as int
                        integer,    // as int
                        command,    // as int
                        menu,       // as int + enum_map
                        binary,     // as int + enem_map (+ branches)
                        indicator,  // as int + branches

                        text        // as text
                       };

SettingType from_string(const std::string &type);
std::string to_string(const SettingType& t);


class SettingMeta
{
public:
  SettingMeta() {}
  SettingMeta(std::string id, SettingType type);
  SettingMeta(std::string id, SettingType type, std::string name);

  SettingMeta stripped() const;
  bool meaningful() const;
  bool numeric() const;

  std::string id() const;
  SettingType type() const;
  bool is(SettingType type) const;

  void set_flag(std::string f);
  bool has_flag(std::string f);
  void remove_flag(std::string f);
  void set_flags(std::initializer_list<std::string> fs);

  void set_enum(int32_t idx, std::string val);
  std::string enum_name(int32_t idx) const;
  std::list<std::string> enum_names() const;
  std::map<int32_t, std::string> enum_map() const
  {
    return enum_map_;
  }

  std::string get_string(std::string name, std::string default_val) const;

  TT T get_num(std::string name, T default_val) const;
  TT void set_val(std::string name, T val);
  TT T min() const;
  TT T max() const;
  TT T step() const;

  std::string value_range() const;
  std::string debug(std::string prepend = "", bool verbose = true) const;

  friend void to_json(json& j, const SettingMeta &s);
  friend void from_json(const json& j, SettingMeta &s);

private:
  std::string                    id_;
  SettingType                    type_ {SettingType::none};
  std::set<std::string>          flags_;
  json                           contents_;
  std::map<int32_t, std::string> enum_map_;

  std::string mins(std::string def) const;
  std::string maxs(std::string def) const;

  TT std::string min_str(std::string def) const;
  TT std::string max_str(std::string def) const;
};


TT std::string SettingMeta::min_str(std::string def) const
{
  auto m = min<T>();
  if (m == -std::numeric_limits<T>::max())
    return def;
  std::stringstream ss;
  ss << m;
  return ss.str();
}

TT std::string SettingMeta::max_str(std::string def) const
{
  auto m = max<T>();
  if (m == std::numeric_limits<T>::max())
    return def;
  std::stringstream ss;
  ss << m;
  return ss.str();
}


TT T SettingMeta::get_num(std::string name, T default_val) const
{
  if (contents_.count(name) && contents_.at(name).is_number())
    return contents_.at(name).get<T>();
  return default_val;
}

TT void SettingMeta::set_val(std::string name, T val)
{
  contents_[name] = val;
}

TT T SettingMeta::min() const
{
  return get_num("min", -std::numeric_limits<T>::max());
}

TT T SettingMeta::max() const
{
  return get_num("max", std::numeric_limits<T>::max());
}

TT T SettingMeta::step() const
{
  return get_num("step", T(1));
}

}

#undef TT
