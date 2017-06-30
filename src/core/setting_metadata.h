#pragma once

#include <string>
#include <map>
#include <set>
#include "json.hpp"
using namespace nlohmann;

namespace DAQuiri {

enum class SettingType {none,
                        stem,       // as branches
                        boolean,    // as int
                        integer,    // as int
                        command,    // as int
                        int_menu,   // as int + branches
                        binary,     // as int + branches
                        indicator,  // as int + branches
                        floating,   // as double
                        floating_precise, // as PreciseFloat
                        text,          // as text
                        color,         // as text
                        file_path,     // as text
                        dir_path,      // as text
                        detector,      // as text DOES NOT SCALE
                        time,          // as ptime
                        time_duration, // as time_duration
                        pattern        // as Pattern
                       };

SettingType to_type(const std::string &type);
std::string to_string(SettingType);


struct SettingMeta
{
  std::string        id_;
  SettingType        setting_type {SettingType::none};

  json               contents;

  bool               writable    {false};
  bool               visible     {true};
  bool               saveworthy  {true};
  double             minimum     {std::numeric_limits<double>::min()};
  double             maximum     {std::numeric_limits<double>::max()};
  double             step        {1};
  int16_t            max_indices {0};
  int64_t            address     {-1};
  std::string        name, description;
  std::string        unit;                       //or extension if file
  std::map<int32_t, std::string> int_menu_items; //or intrinsic branches
  std::set<std::string> flags;

  SettingMeta() {}

  SettingMeta stripped() const;
  bool meaningful() const;

  bool is_numeric() const;
  std::string value_range() const;

  std::string debug(std::string prepend = std::string()) const;

  //deprecate!!!!
  bool operator!= (const SettingMeta& other) const;
  bool operator== (const SettingMeta& other) const;
};

void to_json(json& j, const SettingMeta &s);
void from_json(const json& j, SettingMeta &s);

}
