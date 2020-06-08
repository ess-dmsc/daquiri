#pragma once

#include <core/Detector.h>

namespace DAQuiri {

class ConsumerMetadata
{
public:
  ConsumerMetadata();
  ConsumerMetadata(std::string tp, std::string descr);

  ConsumerMetadata prototype() const;

  // attributes
  Setting attributes() const;
  std::list<Setting> attributes_flat() const;
  Setting get_attribute(std::string setting) const;
  Setting get_attribute(std::string setting, int32_t idx) const;
  Setting get_attribute(Setting setting) const;
  void replace_attribute(const Setting &setting, bool greedy = false);
  void set_attribute(const Setting &setting, bool greedy = false);
  void set_attributes(const std::list<Setting> &s, bool greedy = false);
  void overwrite_all_attributes(Setting settings);

  //read only
  std::string type() const {return type_;}
  std::string type_description() const {return type_description_;}


  void disable_presets();
  void set_det_limit(uint16_t limit);
  bool chan_relevant(uint16_t chan) const;

  std::string debug(std::string prepend, bool verbose = true) const;

  bool shallow_equals(const ConsumerMetadata& other) const;
  bool operator!= (const ConsumerMetadata& other) const;
  bool operator== (const ConsumerMetadata& other) const;

  friend void to_json(json& j, const ConsumerMetadata &s);
  friend void from_json(const json& j, ConsumerMetadata &s);

private:
  //this stuff from factory, immutable upon initialization
  std::string type_;
  std::string type_description_;

  //can change these
  Setting attributes_ {SettingMeta("Attributes", SettingType::stem)};

public:
  std::vector<Detector> detectors;

};

}
