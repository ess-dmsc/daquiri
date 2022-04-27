#include <core/Detector.h>
#include <core/util/ascii_tree.h>

namespace DAQuiri {

Detector::Detector()
{
  settings_ = Setting(SettingMeta("", SettingType::stem));
}

Detector::Detector(std::string id)
  : Detector()
{
  id_ = id;
}

std::string Detector::id() const
{
  return id_;
}

std::string Detector::type() const
{
  return type_;
}

std::list<Setting> Detector::optimizations() const
{
  return settings_.branches.data();
}

void Detector::set_id(const std::string& n)
{
  id_ = n;
}

void Detector::set_type(const std::string& t)
{
  type_ = t;
}

void Detector::add_optimizations(const std::list<Setting>& l,
                                 bool writable_only)
{
  for (auto s : l)
  {
    if (writable_only && s.metadata().has_flag("readonly"))
      continue;
    s.clear_indices();
    settings_.branches.replace(s);
  }
}

Setting Detector::get_setting(std::string id) const
{
  return settings_.find(Setting(id), Match::id);
}

void Detector::clear_optimizations()
{
  settings_ = Setting(SettingMeta("Optimizations", SettingType::stem));
}

void Detector::set_calibration(const Calibration& c)
{
  calibrations_.replace(c);
}

Calibration Detector::get_calibration(CalibID from, CalibID to) const
{
  Calibration ret;
  for (auto c : calibrations_)
  {
    if (from.valid() && !c.from().compare(from))
      continue;
    if (to.valid() && !c.to().compare(to))
      continue;
    ret = c;
  }
  if (ret.valid())
    return ret;
  return Calibration(from, to);
}

bool Detector::operator== (const Detector& other) const
{
  return ((id_ == other.id_) &&
          (type_ == other.type_) &&
          (settings_ == other.settings_));
}

bool Detector::operator!= (const Detector& other) const
{
  return !operator==(other);
}

std::string Detector::debug(std::string prepend) const
{
  std::stringstream ss;
  ss << id_ << "(" << type_ << ")\n";
  ss << prepend << k_branch_end << settings_.debug(prepend + "  ");
  return ss.str();
}

void to_json(json& j, const Detector &s)
{
  j = s.to_json(true);
}

json Detector::to_json(bool options) const
{
  json j;
  j["id"] = id_;
  j["type"] = type_;
  if (options && !settings_.branches.empty())
    j["optimizations"] = settings_;
  return j;
}

void from_json(const json& j, Detector &s)
{
  if (j.count("id"))
    s.id_ = j["id"];
  if (j.count("type"))
    s.type_ = j["type"];
  if (j.count("optimizations"))
    s.settings_ = j["optimizations"];
}

}
