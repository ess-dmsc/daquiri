#pragma once

#include "setting.h"

namespace DAQuiri {

class Detector
{
private:
  std::string name_ {"unknown"};
  std::string type_ {"unknown"};

  Setting settings_ {"Optimization"};

public:
  Detector();
  Detector(std::string name);
  std::string debug(std::string prepend) const;

  std::string name() const;
  void set_name(const std::string&);

  std::string type() const;
  void set_type(const std::string&);

  void clear_optimizations();
  std::list<Setting> optimizations() const;
  Setting get_setting(std::string id) const;
  void add_optimizations(const std::list<Setting>&,
                         bool writable_only = false);

  bool shallow_equals(const Detector& other) const {return (name_ == other.name_);}
  bool operator== (const Detector& other) const;
  bool operator!= (const Detector& other) const;
  
  friend void to_json(json& j, const Detector &s);
  friend void from_json(const json& j, Detector &s);
  json to_json(bool options) const;
};

}
