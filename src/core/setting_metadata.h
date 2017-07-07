#pragma once

#include <string>
#include <map>
#include <set>
#include <list>
#include "json.hpp"
using namespace nlohmann;

#define TT template<typename T>

namespace DAQuiri {

enum class SettingType {none,
                        stem,       // as branches
                        boolean,    // as int
                        integer,    // as int
                        command,    // as int
                        floating,   // as double
                        precise,    // as PreciseFloat
                        menu,       // as int + enum_map
                        binary,     // as int + enem_map (+ branches)
                        indicator,  // as int + branches
                        text,          // as text
                        color,         // as text
                        file,          // as text
                        dir,           // as text
                        detector,      // as text DOES NOT SCALE
                        time,          // as ptime
                        duration,      // as time_duration
                        pattern        // as Pattern
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

  std::string get_string(std::string name, std::string default_val) const;

  TT T get_num(std::string name, T default_val) const;
  TT void set_val(std::string name, T val);
  TT T min() const;
  TT T max() const;
  TT T step() const;

  std::string value_range() const;
  std::string debug(std::string prepend = "") const;

  friend void to_json(json& j, const SettingMeta &s);
  friend void from_json(const json& j, SettingMeta &s);

private:
  std::string                    id_;
  SettingType                    type_ {SettingType::none};
  std::set<std::string>          flags_;
  json                           contents_;
  std::map<int32_t, std::string> enum_map_;
};


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
  return get_num("min", std::numeric_limits<T>::min());
}

TT T SettingMeta::max() const
{
  return get_num("max", std::numeric_limits<T>::max());
}

TT T SettingMeta::step() const
{
  return get_num("step", T(1));
}

//  bool               writable    {true}; changed!!!
//  bool               visible     {true};
//  bool               saveworthy  {true};

}

#undef TT
