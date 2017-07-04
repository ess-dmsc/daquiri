#pragma once

#include "setting_metadata.h"
#include <boost/date_time.hpp>
#include "pattern.h"
#include "precise_float.h"
#include "container.h"

namespace DAQuiri {

enum Match {
  id      = 1 << 0,
  name    = 1 << 1,
  address = 1 << 2,
  indices = 1 << 3
};

inline Match operator|(Match a, Match b) {return static_cast<Match>(static_cast<int>(a) | static_cast<int>(b));}
inline Match operator&(Match a, Match b) {return static_cast<Match>(static_cast<int>(a) & static_cast<int>(b));}

struct Setting;

struct Setting
{
  std::string       id_;
  SettingMeta       metadata;
  std::set<int32_t> indices;

  int64_t                          value_int     {0};
  double                           value_dbl     {0.0};
  PreciseFloat                     value_precise {0};
  std::string                      value_text;
  boost::posix_time::ptime         value_time;
  boost::posix_time::time_duration value_duration;
  Pattern                          value_pattern;

  Container<Setting> branches;

  
  Setting() {}
  Setting(std::string id);
  Setting(SettingMeta meta);

  explicit operator bool() const;
  bool shallow_equals(const Setting& other) const;
  bool operator== (const Setting& other) const;
  bool operator!= (const Setting& other) const;
  bool compare(const Setting &other, Match flags) const;

  void set_value(const Setting &other);
  bool set_setting_r(const Setting &setting, Match flags);
  Setting get_setting(Setting address, Match flags) const;
  void del_setting(Setting address, Match flags);
  bool has(Setting address, Match flags) const;
  std::list<Setting> find_all(const Setting &setting, Match flags) const;
  void set_all(const std::list<Setting> &settings, Match flags);

  void condense();
  void enable_if_flag(bool enable, const std::string &flag);
  void cull_hidden();
  void cull_readonly();
  void strip_metadata();
  void enrich(const std::map<std::string, SettingMeta> &, bool impose_limits = false);
  void enforce_limits();

  std::string debug(std::string prepend = std::string()) const;
  std::string val_to_pretty_string() const;
  std::string indices_to_string(bool showblanks = false) const;

  void set_text(std::string val);
  std::string text() const;

  //Numerics only (float, integer, floatprecise)
  bool numeric() const;
  double number();
  void set_number(double);

  // prefix
  Setting& operator++();
  Setting& operator--();

  // postfix
  Setting operator++(int);
  Setting operator--(int);

  friend void to_json(json& j, const Setting &s);
  friend void from_json(const json& j, Setting &s);

private:
  std::string val_to_string() const;

  bool retrieve_one_setting(Setting&, const Setting&, Match flags) const;
  void delete_one_setting(const Setting&, Setting&, Match flags);

  json val_to_json() const;
  void val_from_json(const json &j);
};

}
