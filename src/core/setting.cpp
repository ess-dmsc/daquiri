#include "setting.h"
#include <boost/lexical_cast.hpp>
#include "bin_hex_print.h"
#include "time_extensions.h"
#include "util.h"

namespace DAQuiri {

Setting::Setting(std::string id)
  : Setting(SettingMeta(id, SettingType::none))
{}

Setting::Setting(SettingMeta meta)
{
  id_ = meta.id();
  metadata = meta;
}

Setting::operator bool() const
{
  return ((*this) != Setting());
}

void Setting::set_time(boost::posix_time::ptime v)
{
  value_time = v;
}

boost::posix_time::ptime Setting::time() const
{
  return value_time;
}

void Setting::set_duration(boost::posix_time::time_duration v)
{
  value_duration = v;
}

boost::posix_time::time_duration Setting::duration() const
{
  return value_duration;
}

bool Setting::compare(const Setting &other, Match flags) const
{
  if ((flags & Match::id) && (id_ != other.id_))
    return false;
  if (flags & Match::indices)
  {
    bool mt = indices.empty() && other.indices.empty();
    for (auto &q : other.indices)
    {
      if (indices.count(q))
      {
        mt = true;
        break;
      }
    }
    if (!mt)
      return false;
  }
  if ((flags & Match::name)
      && (other.metadata.get_string("name", "")
          != metadata.get_string("name", "")))
    return false;
  if ((flags & Match::address)
      && (other.metadata.get_num<size_t>("address", -1)
          != metadata.get_num<size_t>("address", -1)))
    return false;
  return true;
}

bool Setting::shallow_equals(const Setting& other) const
{
  return compare(other, Match::id);
}

bool Setting::operator!= (const Setting& other) const
{
  return !operator==(other);
}

bool Setting::operator== (const Setting& other) const
{
  if (id_            != other.id_) return false;
  if (indices        != other.indices) return false;
  if (value_dbl      != other.value_dbl) return false;
  if (value_int      != other.value_int) return false;
  if (value_text     != other.value_text) return false;
  if (value_time     != other.value_time) return false;
  if (value_duration != other.value_duration) return false;
  if (value_precise  != other.value_precise) return false;
  if (value_pattern  != other.value_pattern) return false;
  if (branches       != other.branches) return false;
  //    if (metadata != other.metadata) return false;
  return true;
}


std::string Setting::indices_to_string(bool showblanks) const
{
  if (!showblanks && indices.empty())
    return "";

  auto max = metadata.get_num<size_t>("max_indices", 0);
  bool valid = false;
  std::string ret;

  if (!indices.empty() || (showblanks && (max > 0)))
  {
    int have = 0;
    ret = "{ ";
    //better dealing with index = -1
    for (auto &q : indices)
    {
      ret += std::to_string(q) + " ";
      if (q >= 0)
      {
        valid = true;
        have++;
      }
    }
    if (showblanks)
    {
      while (have < max)
      {
        valid = true;
        ret += "- ";
        have++;
      }
    }

    ret += "}";
  }
  if (valid)
    return ret;
  else
    return "";
}


std::string Setting::val_to_string() const
{
  std::stringstream ss;
  if (metadata.type() == SettingType::boolean) {
    if (value_int != 0)
      ss << "True";
    else
      ss << "False";
  }
  else if ((metadata.type() == SettingType::integer) ||
           (metadata.type() == SettingType::menu) ||
           (metadata.type() == SettingType::indicator) )
    ss << std::to_string(value_int);
  else if (metadata.type() == SettingType::binary)
    //    ss << itohex64(value_int);
    ss << std::to_string(value_int);
  else if (metadata.type() == SettingType::floating)
    ss << std::setprecision(std::numeric_limits<double>::max_digits10) << value_dbl;
  else if (metadata.type() == SettingType::precise)
    ss << std::setprecision(std::numeric_limits<PreciseFloat>::max_digits10) << value_precise;
  else if (metadata.type() == SettingType::pattern)
    ss << value_pattern.debug();
  else if ((metadata.type() == SettingType::text) ||
           (metadata.type() == SettingType::color) ||
           (metadata.type() == SettingType::detector) ||
           (metadata.type() == SettingType::file) ||
           (metadata.type() == SettingType::dir))
    ss << value_text;
  else if (metadata.type() == SettingType::time)
  {
    if (value_time.is_not_a_date_time())
      ss << "INVALID";
    else
      ss << boost::posix_time::to_iso_extended_string(value_time);
  }
  else if (metadata.type() == SettingType::duration)
    ss << boost::posix_time::to_simple_string(value_duration);
  return ss.str();
}

std::string Setting::val_to_pretty_string() const
{
  if ((metadata.type() == SettingType::duration)
      && !value_duration.is_not_a_date_time())
    return very_simple(value_duration);
  else if (metadata.type() == SettingType::menu)
    return metadata.enum_name(value_int);
  else if (metadata.type() == SettingType::indicator)
  {
    auto s = get_setting(Setting(metadata.enum_name(value_int)), Match::id);
    return s.metadata.get_string("name", "");
  }
  else if (metadata.type() == SettingType::binary)
  {
    auto size = metadata.get_num<size_t>("bits", 16);
    if (size > 32)
      return "0x" + itohex64(value_int);
    else if (size > 16)
      return "0x" + itohex32(value_int);
    else
      return "0x" + itohex16(value_int);
  } else if (metadata.type() == SettingType::stem)
    return value_text;
  return val_to_string();
}


std::string Setting::debug(std::string prepend) const
{
  std::string ret = id_;
  if (!indices.empty())
    ret += indices_to_string(true);
  ret += "=" + val_to_pretty_string();
  ret += " " + metadata.debug(prepend);
  if (!branches.empty())
    ret += " branches=" + std::to_string(branches.size());
  ret += "\n";
  if (!branches.empty())
    for (size_t i = 0; i < branches.size(); ++i)
    {
      if ((i+1) == branches.size())
        ret += prepend + k_branch_end + branches.get(i).debug(prepend + "  ");
      else
        ret += prepend + k_branch_mid + branches.get(i).debug(prepend + k_branch_pre);
    }
  return ret;
}

void Setting::set_value(const Setting &other)
{
  value_int = other.value_int;
  value_text = other.value_text;
  value_dbl = other.value_dbl;
  value_time = other.value_time;
  value_duration = other.value_duration;
  value_precise = other.value_precise;
  value_pattern = other.value_pattern;
}

bool Setting::set_setting_r(const Setting &setting, Match flags)
{
  if (this->compare(setting, flags))
  {
    this->set_value(setting);
    return true;
  } else if ((this->metadata.type() == SettingType::stem)
             || (this->metadata.type() == SettingType::indicator)) {
    for (auto &q : this->branches.my_data_)
      if (q.set_setting_r(setting, flags))
        return true;
  }
  return false;
}


bool Setting::retrieve_one_setting(Setting &det, const Setting& root,
                                   Match flags) const
{
  if (root.compare(det, flags)) {
    det = root;
    return true;
  } else if ((root.metadata.type() == SettingType::stem)
             || (root.metadata.type() == SettingType::indicator)) {
    for (auto &q : root.branches.my_data_)
      if (retrieve_one_setting(det, q, flags))
        return true;
  }
  return false;
}

Setting Setting::get_setting(Setting address, Match flags) const
{
  if (retrieve_one_setting(address, *this, flags))
    return address;
  else
    return Setting();
}

std::list<Setting> Setting::find_all(const Setting &setting, Match flags) const
{
  std::list<Setting> result;
  if (metadata.type() == SettingType::stem)
    for (auto &q : branches.my_data_)
      result.splice(result.end(), q.find_all(setting, flags));
  else if (compare(setting, flags) && (metadata.type() != SettingType::detector))
    result.push_back(*this);
  return result;
}

void Setting::set_all(const std::list<Setting> &settings, Match flags)
{
  if (metadata.type() == SettingType::stem)
    for (auto &q : branches.my_data_)
      q.set_all(settings, flags);
  else
    for (auto &q : settings)
      if (compare(q, flags) && (metadata.type() != SettingType::detector))
        set_value(q);
}

bool Setting::has(Setting address, Match flags) const
{
  return (retrieve_one_setting(address, *this, flags));
}

void Setting::delete_one_setting(const Setting &det, Setting& root, Match flags)
{
  Setting truncated = root;
  truncated.branches.clear();
  for (auto &q : root.branches.my_data_)
  {
    if (!q.compare(det, flags))
    {
      if (q.metadata.type() == SettingType::stem)
      {
        delete_one_setting(det, q, flags);
        if (!q.branches.empty())
          truncated.branches.add_a(q);
      }
      else
        truncated.branches.add_a(q);
    }
  }
  root = truncated;
}

void Setting::del_setting(Setting address, Match flags)
{
  Setting addy(address);
  delete_one_setting(addy, *this, flags);
}

void Setting::enrich(const std::map<std::string, SettingMeta> &setting_definitions,
                     bool impose_limits)
{
  if (setting_definitions.count(id_) > 0)
  {
    metadata = setting_definitions.at(id_);
    if ((metadata.type() == SettingType::indicator) ||
        (metadata.type() == SettingType::binary) ||
        (metadata.type() == SettingType::stem))
    {
      Container<Setting> br = branches;
      branches.clear();
      for (auto id : metadata.enum_names())
      {
        if (setting_definitions.count(id))
        {
          SettingMeta newmeta = setting_definitions.at(id);
          for (size_t i=0; i < br.size(); ++i)
          {
            Setting newset = br.get(i);
            if (newset.id_ == newmeta.id())
            {
              newset.enrich(setting_definitions, impose_limits);
              branches.add_a(newset);
            }
          }
        }
      }
      for (size_t i=0; i < br.size(); ++i)
      {
        if (setting_definitions.count(br.get(i).id_) == 0)
        {
          Setting newset = br.get(i);
          newset.metadata.set_flag("hidden");
          branches.add_a(newset);
        }
      }
    }
    else if (impose_limits)
      enforce_limits();
  }
}

void Setting::enforce_limits()
{
  if (metadata.type() == SettingType::integer)
  {
    value_int = std::min(metadata.max<int64_t>(), value_int);
    value_int = std::max(metadata.min<int64_t>(), value_int);
  }
  else if (metadata.type() == SettingType::floating)
  {
    value_dbl = std::min(metadata.max<double>(), value_dbl);
    value_dbl = std::max(metadata.min<double>(), value_dbl);
  }
  else if (metadata.type() == SettingType::precise)
  {
    value_precise = std::min(metadata.max<PreciseFloat>(), value_precise);
    value_precise = std::max(metadata.min<PreciseFloat>(), value_precise);
  }
  else if (metadata.type() == SettingType::pattern)
  {
    auto size = metadata.get_num<size_t>("chans", 0);
    if (value_pattern.gates().size() != size)
      value_pattern.resize(size);
  }
}


void Setting::condense()
{
  Container<Setting> oldbranches = branches;
  branches.clear();
  for (size_t i=0; i < oldbranches.size(); ++i) {
    Setting setting = oldbranches.get(i);
    if (setting.metadata.type() == SettingType::stem)
    {
      setting.condense();
      branches.add_a(setting);
    }
    else if ((setting.metadata.type() != SettingType::command)
             && !setting.metadata.has_flag("readonly")
             && !metadata.has_flag("dontsave"))
    {
      branches.add_a(setting);
    }
  }
}

void Setting::enable_if_flag(bool enable, const std::string &flag)
{
  if (metadata.has_flag(flag))
  {
    if (!enable)
      metadata.set_flag("readonly");
    else
      metadata.remove_flag("readonly");
  }
  if ((metadata.type() == SettingType::stem) && !branches.empty())
    for (auto &q : branches.my_data_)
      q.enable_if_flag(enable, flag);
}

void Setting::cull_hidden()
{
  Container<Setting> oldbranches = branches;
  branches.clear();
  for (size_t i=0; i < oldbranches.size(); ++i) {
    Setting setting = oldbranches.get(i);
    if (!setting.metadata.has_flag("hidden"))
    {
      if (setting.metadata.type() == SettingType::stem)
      {
        setting.cull_hidden();
        if (!setting.branches.empty())
          branches.add_a(setting);
      }
      else
        branches.add_a(setting);
    }
  }
}

void Setting::cull_readonly()
{
  Container<Setting> oldbranches = branches;
  branches.clear();
  for (size_t i=0; i < oldbranches.size(); ++i)
  {
    Setting setting = oldbranches.get(i);
    if (setting.metadata.type() == SettingType::stem)
    {
      setting.cull_readonly();
      if (!setting.branches.empty())
        branches.add_a(setting);
    }
    else if (!setting.metadata.has_flag("readonly"))
      branches.add_a(setting);
  }
}

void Setting::strip_metadata()
{
  for (auto &q : branches.my_data_)
    q.strip_metadata();
  metadata = metadata.stripped();
}

//For numerics
bool Setting::numeric() const
{
  return metadata.numeric();
}

void Setting::set_text(std::string val)
{
  value_text = val;
}

std::string Setting::get_text() const
{
  return value_text;
}

double Setting::get_number()
{
  if (metadata.type() == SettingType::integer)
    return static_cast<double>(value_int);
  else if (metadata.type() == SettingType::floating)
    return value_dbl;
  else if (metadata.type() == SettingType::precise)
    //    return value_precise.convert_to<double>();
    return static_cast<double>(value_precise);
  return std::numeric_limits<double>::quiet_NaN();
}

void Setting::set_number(double val)
{
  if (metadata.type() == SettingType::integer)
    value_int = val;
  else if (metadata.type() == SettingType::floating)
    value_dbl = val;
  else if (metadata.type() == SettingType::precise)
    value_precise = val;
  enforce_limits();
}

// prefix
Setting& Setting::operator++()
{
  if (metadata.type() == SettingType::integer)
  {
    value_int += metadata.step<int64_t>();
    if (value_int > metadata.max<int64_t>())
      value_int = metadata.max<int64_t>();
  }
  else if (metadata.type() == SettingType::floating)
  {
    value_dbl += metadata.step<int64_t>();
    if (value_dbl > metadata.max<int64_t>())
      value_dbl = metadata.max<int64_t>();
  }
  else if (metadata.type() == SettingType::precise)
  {
    value_precise += metadata.step<int64_t>();
    if (value_precise > metadata.max<int64_t>())
      value_precise = metadata.max<int64_t>();
  }
  return *this;
}

Setting& Setting::operator--()
{
  if (metadata.type() == SettingType::integer)
  {
    value_int -= metadata.step<int64_t>();
    if (value_int < metadata.min<int64_t>())
      value_int = metadata.min<int64_t>();
  }
  else if (metadata.type() == SettingType::floating)
  {
    value_dbl -= metadata.step<int64_t>();
    if (value_dbl < metadata.min<int64_t>())
      value_dbl = metadata.min<int64_t>();
  }
  else if (metadata.type() == SettingType::precise)
  {
    value_precise -= metadata.step<int64_t>();
    if (value_precise < metadata.min<int64_t>())
      value_precise = metadata.min<int64_t>();
  }
  return *this;
}

// postfix
Setting Setting::operator++(int)
{
  Setting tmp(*this);
  operator++();
  return tmp;
}

Setting Setting::operator--(int)
{
  Setting tmp(*this);
  operator--();
  return tmp;
}

json Setting::val_to_json() const
{
  if (metadata.type() == SettingType::boolean)
    return json(bool(value_int));
  else if ((metadata.type() == SettingType::integer) ||
           (metadata.type() == SettingType::menu) ||
           (metadata.type() == SettingType::binary) ||
           (metadata.type() == SettingType::indicator) )
    return json(value_int);
  else if (metadata.type() == SettingType::pattern)
    return json(value_pattern);
  else if (metadata.type() == SettingType::floating)
    return json(value_dbl);
  else if (metadata.type() == SettingType::precise)
    return json(value_precise);
  else
    return json(val_to_string());
}

void Setting::val_from_json(const json &j)
{
  if (metadata.type() == SettingType::boolean)
    value_int = j.get<bool>();
  else if ((metadata.type() == SettingType::integer) ||
           (metadata.type() == SettingType::menu) ||
           (metadata.type() == SettingType::binary) ||
           (metadata.type() == SettingType::indicator) )
    value_int = j.get<int64_t>();
  else if (metadata.type() == SettingType::pattern)
    value_pattern = j;
  else if (metadata.type() == SettingType::floating)
    value_dbl = j.get<double>();
  else if (metadata.type() == SettingType::precise)
    value_precise = j;
  else if (metadata.type() == SettingType::time)
    value_time = from_iso_extended(j.get<std::string>());
  else if (metadata.type() == SettingType::duration)
    value_duration = boost::posix_time::duration_from_string(j.get<std::string>());
  else if ((metadata.type() == SettingType::text) ||
           (metadata.type() == SettingType::detector) ||
           (metadata.type() == SettingType::color) ||
           (metadata.type() == SettingType::file) ||
           (metadata.type() == SettingType::dir))
    value_text = j.get<std::string>();
}


void to_json(json& j, const Setting &s)
{
  j["id"] = s.id_;
  j["type"] = to_string(s.metadata.type());

  if (s.metadata.meaningful())
    j["metadata"] = s.metadata;

  if (!s.indices.empty())
    j["indices"] = s.indices;

  if (s.metadata.type() == SettingType::stem)
  {
    j["branches"] = s.branches;
    if (!s.value_text.empty())
      j["reference"] = s.value_text;
  }
  else
    j["value"] = s.val_to_json();
}

void from_json(const json& j, Setting &s)
{
  s.id_ = j["id"];
  s.metadata = SettingMeta(s.id_, from_string(j["type"]));

  if (j.count("metadata"))
    s.metadata = j["metadata"];

  if (j.count("indices"))
    for (auto it : j["indices"])
      s.indices.insert(it.get<int32_t>());

  if (s.metadata.type() == SettingType::stem)
  {
    if (j.count("reference"))
      s.value_text = j["reference"];
    s.branches = j["branches"];
  } else
    s.val_from_json(j["value"]);
}


}


