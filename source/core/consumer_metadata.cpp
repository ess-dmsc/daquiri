#include <core/consumer_metadata.h>
#include <core/util/ascii_tree.h>

namespace DAQuiri {

ConsumerMetadata::ConsumerMetadata() {}

ConsumerMetadata::ConsumerMetadata(std::string tp,
                                   std::string descr)
    : type_(tp), type_description_(descr) {}

ConsumerMetadata ConsumerMetadata::prototype() const
{
  auto ret = *this;

  ret.attributes_.enable_if_flag(true, "preset");
  ret.attributes_.cull_readonly();

  return ret;
}

bool ConsumerMetadata::shallow_equals(const ConsumerMetadata& other) const
{
  return operator==(other);
}

bool ConsumerMetadata::operator!=(const ConsumerMetadata& other) const
{
  return !operator==(other);
}

bool ConsumerMetadata::operator==(const ConsumerMetadata& other) const
{
  if (type_ != other.type_) return false; //assume other type info same
  if (attributes_ != other.attributes_) return false;
  return true;
}

std::string ConsumerMetadata::debug(std::string prepend, bool verbose) const
{
  std::stringstream ss;
  ss << "METADATA\n";
  ss << prepend << k_branch_mid_B << "Type: " << type_ << "\n";
//     << " (dim=" << dimensions_ << ")\n";
  //ss << " " << type_description_;
  if (detectors.size())
  {
    ss << prepend << k_branch_mid_B << "Detectors:\n";
    for (size_t i = 0; i < detectors.size(); ++i)
    {
      if ((i + 1) == detectors.size())
        ss << prepend << k_branch_pre_B << k_branch_end_B << i << ": "
           << detectors.at(i).debug(prepend + k_branch_pre_B + "  ");
      else
        ss << prepend << k_branch_pre_B << k_branch_mid_B << i << ": "
           << detectors.at(i).debug(prepend + k_branch_pre_B + k_branch_pre_B);
    }
  }
  ss << prepend << k_branch_end_B << attributes_.debug(prepend + "  ", verbose);
  return ss.str();
}

Setting ConsumerMetadata::get_attribute(Setting setting) const
{
  return attributes_.find(setting, Match::id | Match::indices);
}

Setting ConsumerMetadata::get_attribute(std::string setting) const
{
  return attributes_.find(Setting(setting), Match::id | Match::indices);
}

Setting ConsumerMetadata::get_attribute(std::string setting, int32_t idx) const
{
  Setting find(setting);
  find.set_indices({idx});
  return attributes_.find(find, Match::id | Match::indices);
}

void ConsumerMetadata::replace_attribute(const Setting& setting, bool greedy)
{
  attributes_.replace(setting, Match::id | Match::indices, greedy);
}

void ConsumerMetadata::set_attribute(const Setting& setting, bool greedy)
{
  attributes_.set(setting, Match::id | Match::indices, greedy);
}

Setting ConsumerMetadata::attributes() const
{
  return attributes_;
}

std::list<Setting> ConsumerMetadata::attributes_flat() const
{
  auto aa = attributes();
  auto aaa = aa.find_all(Setting(), Match(0));
  std::list<Setting> ret;
  for (auto a : aaa)
  {
    if (!a.is(SettingType::stem))
      ret.push_back(a);
  }
  return ret;
}

void ConsumerMetadata::set_attributes(const std::list<Setting>& settings, bool greedy)
{
  for (auto& setting : settings)
    set_attribute(setting, greedy);
}

void ConsumerMetadata::overwrite_all_attributes(Setting settings)
{
  attributes_ = settings;
}

void ConsumerMetadata::disable_presets()
{
  attributes_.enable_if_flag(false, "preset");
}

void to_json(json& j, const ConsumerMetadata& s)
{
  try
  {
    j["type"] = s.type_;

    if (s.attributes_.branches.size())
      j["attributes"] = s.attributes_;

    if (!s.detectors.empty())
      for (auto& d : s.detectors)
        j["detectors"].push_back(d);
  }
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("<ConsumerMetadata> Could not convert to json"
                                                  + s.debug("")));
  }
}

void from_json(const json& j, ConsumerMetadata& s)
{
  try
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
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("<ConsumerMetadata> Could not convert from json"
                                                  + j.dump(0)));
  }
}

void ConsumerMetadata::set_det_limit(uint16_t limit)
{
  if (limit < 1)
    limit = 1;

  for (auto& a : attributes_.branches)
    if (a.metadata().type() == SettingType::pattern)
    {
      auto p = a.pattern();
      p.resize(limit);
      a.set_pattern(p);
    }
    else if (a.metadata().type() == SettingType::stem)
    {
      Setting prototype;
      for (auto& p : a.branches)
        if (p.has_index(-1))
          prototype = p;
      if (prototype.metadata().type() == SettingType::stem)
      {
        a.clear_indices();
        a.branches.clear();
        prototype.hide(true);
        a.branches.add_a(prototype);
        prototype.hide(false);
        for (int32_t i = 0; i < limit; ++i)
        {
          prototype.set_indices({i});
          for (auto& p : prototype.branches)
            p.set_indices({i});
          a.branches.add_a(prototype);
          //          a.indices.insert(i);
        }
      }
    }
}

bool ConsumerMetadata::chan_relevant(uint16_t chan) const
{
  auto allpatterns = attributes_.find_all(Setting({"", SettingType::pattern}), Match::stype);

  for (const Setting& s : allpatterns)
    if ((s.metadata().type() == SettingType::pattern) &&
        (s.pattern().relevant(chan)))
      return true;
  return false;
}

}
