#pragma once

#include <core/calibration/calibration.h>
#include <core/plugin/setting.h>

namespace DAQuiri {

class Detector {
private:
  std::string id_{"unknown"};
  std::string type_{"unknown"};

  Setting settings_{"Optimization"};
  Container<Calibration> calibrations_;

public:
  Detector();
  Detector(std::string id);
  std::string debug(std::string prepend) const;

  std::string id() const;
  void set_id(const std::string &);

  std::string type() const;
  void set_type(const std::string &);

  void clear_optimizations();
  std::list<Setting> optimizations() const;
  Setting get_setting(std::string id) const;
  void add_optimizations(const std::list<Setting> &,
                         bool writable_only = false);

  void set_calibration(const Calibration &);
  Calibration get_calibration(CalibID from, CalibID to) const;

  bool shallow_equals(const Detector &other) const {
    return (id_ == other.id_);
  }
  bool operator==(const Detector &other) const;
  bool operator!=(const Detector &other) const;

  friend void to_json(json &j, const Detector &s);
  friend void from_json(const json &j, Detector &s);
  json to_json(bool options) const;
};

} // namespace DAQuiri
