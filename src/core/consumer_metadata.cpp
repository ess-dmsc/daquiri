#include "consumer_metadata.h"
#include "util.h"

namespace DAQuiri {

ConsumerMetadata::ConsumerMetadata()
{
  attributes_.metadata.setting_type = SettingType::stem;
}

ConsumerMetadata::ConsumerMetadata(std::string tp, std::string descr, uint16_t dim,
                   std::list<std::string> itypes, std::list<std::string> otypes)
  : type_(tp)
  , type_description_(descr)
  , dimensions_(dim)
  , input_types_(itypes)
  , output_types_(otypes)
{
  attributes_.metadata.setting_type = SettingType::stem;
}

bool ConsumerMetadata::shallow_equals(const ConsumerMetadata& other) const
{
  return operator ==(other);
}

bool ConsumerMetadata::operator!= (const ConsumerMetadata& other) const
{
  return !operator==(other);
}

bool ConsumerMetadata::operator== (const ConsumerMetadata& other) const
{
  if (type_ != other.type_) return false; //assume other type info same
  if (attributes_ != other.attributes_) return false;
  return true;
}


std::string ConsumerMetadata::debug(std::string prepend) const
{
  std::stringstream ss;
  ss << type_ << " (dim=" << dimensions_ << ")\n";
  //ss << " " << type_description_;
  ss << prepend << k_branch_mid_B << "Detectors:\n";
  for (size_t i=0; i < detectors.size(); ++i)
  {
    if ((i+1) == detectors.size())
      ss << prepend << k_branch_pre_B << k_branch_end_B << i << ": " << detectors.at(i).debug(prepend + k_branch_pre_B + "  ");
    else
      ss << prepend << k_branch_pre_B << k_branch_mid_B << i << ": " << detectors.at(i).debug(prepend + k_branch_pre_B + k_branch_pre_B);
  }
  ss << prepend << k_branch_end_B << attributes_.debug(prepend + "  ");
  return ss.str();
}

Setting ConsumerMetadata::get_attribute(Setting setting) const
{
  return attributes_.get_setting(setting, Match::id | Match::indices);
}

Setting ConsumerMetadata::get_attribute(std::string setting) const
{
  return attributes_.get_setting(Setting(setting), Match::id | Match::indices);
}

Setting ConsumerMetadata::get_attribute(std::string setting, int32_t idx) const
{
  Setting find(setting);
  find.indices.insert(idx);
  return attributes_.get_setting(find, Match::id | Match::indices);
}

void ConsumerMetadata::set_attribute(const Setting &setting)
{
  attributes_.set_setting_r(setting, Match::id | Match::indices);
}

Setting ConsumerMetadata::get_all_attributes() const
{
  return attributes_;
}

Setting ConsumerMetadata::attributes() const
{
  return attributes_;
}

void ConsumerMetadata::set_attributes(const Setting &settings)
{
  if (settings.branches.size())
    for (auto &s : settings.branches.my_data_)
      set_attributes(s);
  else if (attributes_.has(settings, Match::id | Match::indices))
    set_attribute(settings);
}

void ConsumerMetadata::overwrite_all_attributes(Setting settings)
{
  attributes_ = settings;
}

void ConsumerMetadata::disable_presets()
{
  attributes_.enable_if_flag(false, "preset");
}

void to_json(json& j, const ConsumerMetadata &s)
{
  j["type"] = s.type_;

  if (s.attributes_.branches.size())
    j["attributes"] = s.attributes_;

  if (!s.detectors.empty())
    for (auto &d : s.detectors)
      j["detectors"].push_back(d);
}

void from_json(const json& j, ConsumerMetadata &s)
{
  s.type_ = j["type"];

  if (j.count("attributes"))
    s.attributes_ = j["attributes"];

  if (j.count("detectors"))
  {
    auto o = j["detectors"];
    for (json::iterator it = o.begin(); it != o.end(); ++it)
      s.detectors.push_back(it.value());
  }
}

void ConsumerMetadata::set_det_limit(uint16_t limit)
{
  if (limit < 1)
    limit = 1;

  for (auto &a : attributes_.branches.my_data_)
    if (a.metadata.setting_type == SettingType::pattern)
      a.value_pattern.resize(limit);
    else if (a.metadata.setting_type == SettingType::stem) {
      Setting prototype;
      for (auto &p : a.branches.my_data_)
        if (p.indices.count(-1))
          prototype = p;
      if (prototype.metadata.setting_type == SettingType::stem) {
        a.indices.clear();
        a.branches.clear();
        prototype.metadata.visible = false;
        a.branches.add_a(prototype);
        prototype.metadata.visible = true;
        for (int i=0; i < limit; ++i) {
          prototype.indices.clear();
          prototype.indices.insert(i);
          for (auto &p : prototype.branches.my_data_)
            p.indices = prototype.indices;
          a.branches.add_a(prototype);
          //          a.indices.insert(i);
        }
      }
    }
}

bool ConsumerMetadata::chan_relevant(uint16_t chan) const
{
  for (const Setting &s : attributes_.branches.my_data_)
    if ((s.metadata.setting_type == SettingType::pattern) &&
        (s.value_pattern.relevant(chan)))
      return true;
  return false;
}


}
