#include <core/plugin/setting.h>
#include <core/util/bin_hex_print.h>
#include <core/util/string_extensions.h>
#include <core/util/time_extensions.h>
#include <core/util/ascii_tree.h>
#include <core/util/color_bash.h>

#include <core/util/custom_logger.h>

namespace DAQuiri {

Setting::Setting(std::string id)
  : Setting(SettingMeta(id, SettingType::none))
{}

Setting::Setting(SettingMeta meta)
{
  metadata_ = meta;
}

Setting::operator bool() const
{
  return ((*this) != Setting());
}

std::string Setting::id() const
{
  return metadata_.id();
}

SettingMeta Setting::metadata() const
{
  return metadata_;
}

bool Setting::is(SettingType type) const
{
  return (metadata().is(type));
}

void Setting::clear_indices()
{
  indices_.clear();
}

void Setting::set_indices(std::set<int32_t> l)
{
  clear_indices();
  add_indices(l);
}

void Setting::add_indices(std::set<int32_t> l)
{
  for (auto ll : l)
    indices_.insert(ll);
}

bool Setting::has_index(int32_t i) const
{
  return indices_.count(i);
}

std::set<int32_t> Setting::indices() const
{
  return indices_;
}

Setting::Setting(std::string sid, hr_time_t v)
  : Setting(SettingMeta(sid, SettingType::time))
{
  value_time = v;
}

void Setting::set_time(hr_time_t v)
{
  value_time = v;
}

hr_time_t Setting::time() const
{
  return value_time;
}

Setting::Setting(std::string sid, hr_duration_t v)
  : Setting(SettingMeta(sid, SettingType::duration))
{
  value_duration = v;
}

void Setting::set_duration(hr_duration_t v)
{
  value_duration = v;
}

hr_duration_t Setting::duration() const
{
  return value_duration;
}

Setting::Setting(std::string sid, Pattern v)
  : Setting(SettingMeta(sid, SettingType::pattern))
{
  value_pattern = v;
}

void Setting::set_pattern(Pattern v)
{
  value_pattern = v;
}

Pattern Setting::pattern() const
{
  return value_pattern;
}

bool Setting::compare(const Setting &other, Match m) const
{
  if ((m & Match::value) && (other != *this))
    return false;
  if ((m & Match::stype) && !other.is(metadata_.type()))
    return false;
  if ((m & Match::id) && (id() != other.id()))
    return false;
  if ((m & Match::indices) && !compare_indices(other))
    return false;
  if ((m & Match::name)
      && (other.metadata_.get_string("name", "")
          != metadata_.get_string("name", "")))
    return false;
  if ((m & Match::address)
      && (other.metadata_.get_num<int32_t>("address", -1)
          != metadata_.get_num<int32_t>("address", -1)))
    return false;
  return true;
}

bool Setting::compare_indices(const Setting &other) const
{
  // not symmetrical!
  if (indices_.empty() && other.indices_.empty())
    return true;
  for (auto &q : other.indices_)
    if (indices_.count(q))
      return true;
  return false;
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
  if (id()           != other.id()) return false;
  if (indices_       != other.indices_) return false;
  if (value_dbl      != other.value_dbl) return false;
  if (value_int      != other.value_int) return false;
  if (value_text     != other.value_text) return false;
  if (value_time     != other.value_time) return false;
  if (value_duration != other.value_duration) return false;
  if (value_precise  != other.value_precise) return false;
  if (value_pattern  != other.value_pattern) return false;
  if (branches       != other.branches) return false;
  //    if (metadata_ != other.metadata_) return false;
  return true;
}


std::string Setting::indices_to_string(bool showblanks) const
{
  if (!showblanks && indices_.empty())
    return "";

  auto max = metadata_.get_num<size_t>("max_indices", 0);

  std::vector<std::string> idcs;
  for (auto &q : indices_)
    idcs.push_back(std::to_string(q));

  if (showblanks && idcs.size() < max)
    for (auto i = idcs.size(); i < max; ++i)
      idcs.push_back("-");

  if (idcs.size())
    return "{" + join(idcs, " ") + "}";

  return "";
}

void Setting::set_val(const Setting &other)
{
  value_int = other.value_int;
  value_dbl = other.value_dbl;
  value_precise = other.value_precise;
  value_text = other.value_text;
  value_time = other.value_time;
  value_duration = other.value_duration;
  value_pattern = other.value_pattern;
}

bool Setting::set_first(const Setting &setting, Match m)
{
  if (this->compare(setting, m))
  {
    this->set_val(setting);
    return true;
  }
  for (auto &q : this->branches)
    if (q.set_first(setting, m))
      return true;
  return false;
}

void Setting::set_all(const Setting &setting, Match m)
{
  if (this->compare(setting, m))
    this->set_val(setting);
  for (auto &q : this->branches)
    q.set_all(setting, m);
}

bool Setting::replace_first(const Setting &setting, Match m)
{
  if (this->compare(setting, m))
  {
    *this = setting;
    return true;
  }
  for (auto &q : this->branches)
    if (q.replace_first(setting, m))
      return true;
  return false;
}

void Setting::replace_all(const Setting &setting, Match m)
{
  if (this->compare(setting, m))
    *this = setting;
  for (auto &q : this->branches)
    q.replace_all(setting, m);
}

void Setting::replace(const Setting &s, Match m, bool greedy)
{
  if (greedy)
    replace_all(s, m);
  else
    replace_first(s, m);
}

void Setting::set(const Setting &s, Match m, bool greedy)
{
  if (greedy)
    set_all(s, m);
  else
    set_first(s, m);
}

void Setting::set(const std::list<Setting> &s, Match m, bool greedy)
{
  if (greedy)
    for (const auto& ss : s)
      set_all(ss, m);
  else
    for (const auto& ss : s)
      set_first(ss, m);
}


bool Setting::find_dfs(Setting &result, const Setting& root,
                       Match m) const
{
  for (auto &q : root.branches)
    if (find_dfs(result, q, m))
      return true;
  if (root.compare(result, m))
  {
    result = root;
    return true;
  }
  return false;
}

Setting Setting::find(Setting address, Match m) const
{
  if (find_dfs(address, *this, m))
    return address;
  else
    return Setting();
}

std::list<Setting> Setting::find_all(const Setting &setting, Match m) const
{
  std::list<Setting> result;
  for (auto &q : branches)
    result.splice(result.end(), q.find_all(setting, m));
  if (compare(setting, m))
    result.push_back(*this);
  return result;
}

bool Setting::has(Setting address, Match m) const
{
  return (find_dfs(address, *this, m));
}

void Setting::erase_matches(const Setting &address, Setting& root, Match m)
{
  Setting truncated = root;
  truncated.branches.clear();
  for (auto &q : root.branches)
  {
    if (q.compare(address, m))
      continue;
    erase_matches(address, q, m);
    truncated.branches.add_a(q);
  }
  root = truncated;
}

void Setting::erase(Setting address, Match m)
{
  Setting addy(address);
  erase_matches(addy, *this, m);
}

void Setting::enrich(const std::map<std::string, SettingMeta> &setting_definitions,
                     bool impose_limits)
{
  if (setting_definitions.count(id()) > 0)
  {
    metadata_ = setting_definitions.at(id());
    auto idm = metadata_.enum_map();

    if (idm.size())
    {
      Container<Setting> new_branches;
      auto idss = metadata_.enum_names();
      std::set<std::string> ids(idss.begin(), idss.end());
      for (auto old : branches)
      {
        if (setting_definitions.count(old.id()))
          old.enrich(setting_definitions, impose_limits);
        else
          old.hide(true);
        new_branches.add_a(old);
        if (ids.count(old.id()))
          ids.erase(old.id());
      }

      for (auto id : idm)
      {
        if (!ids.count(id.second))
          continue;
        if (!setting_definitions.count(id.second))
          continue;
        Setting newset = Setting(setting_definitions.at(id.second));
        newset.enrich(setting_definitions, impose_limits);
        newset.indices_ = indices_;
        new_branches.add_a(newset);
      }

      branches = new_branches;
    }
    else if (impose_limits)
      enforce_limits();
  }
}

void Setting::hide(bool h)
{
  if (h)
    metadata_.set_flag("hidden");
  else
    metadata_.remove_flag("hidden");
}

void Setting::enforce_limits()
{
  if (is(SettingType::integer))
  {
    value_int = std::min(metadata_.max<integer_t>(), value_int);
    value_int = std::max(metadata_.min<integer_t>(), value_int);
  }
  else if (is(SettingType::floating))
  {
    value_dbl = std::min(metadata_.max<floating_t>(), value_dbl);
    value_dbl = std::max(metadata_.min<floating_t>(), value_dbl);
  }
  else if (is(SettingType::precise))
  {
    value_precise = std::min(metadata_.max<precise_t>(), value_precise);
    value_precise = std::max(metadata_.min<precise_t>(), value_precise);
  }
  else if (is(SettingType::pattern))
  {
    auto size = metadata_.get_num<size_t>("chans", 0);
    if (value_pattern.gates().size() != size)
      value_pattern.resize(size);
  }
}


void Setting::condense()
{
  Container<Setting> new_branches;
  for (auto setting : branches)
  {
    if (setting.is(SettingType::stem))
    {
      setting.condense();
      new_branches.add_a(setting);
    }
    else if (!setting.is(SettingType::command)
             && !setting.metadata_.has_flag("readonly")
             && !metadata_.has_flag("dontsave"))
      new_branches.add_a(setting);
  }
  branches = new_branches;
}

void Setting::enable_if_flag(bool enable, const std::string &flag)
{
  if (flag.empty() || metadata_.has_flag(flag))
  {
    if (!enable)
      metadata_.set_flag("readonly");
    else
      metadata_.remove_flag("readonly");
  }
  if (is(SettingType::stem) && !branches.empty())
    for (auto &q : branches)
      q.enable_if_flag(enable, flag);
}

void Setting::cull_hidden()
{
  Container<Setting> new_branches;
  for (auto setting : branches)
  {
    if (setting.metadata_.has_flag("hidden"))
      continue;
    else if (setting.is(SettingType::stem))
    {
      setting.cull_hidden();
      if (!setting.branches.empty())
        new_branches.add_a(setting);
    }
    else
      new_branches.add_a(setting);
  }
  branches = new_branches;
}

void Setting::cull_readonly()
{
  Container<Setting> new_branches;
  for (auto setting : branches)
  {
    if (setting.is(SettingType::stem))
    {
      setting.cull_readonly();
      if (!setting.branches.empty())
        new_branches.add_a(setting);
    }
    else if (!setting.metadata_.has_flag("readonly"))
      new_branches.add_a(setting);
  }
  branches = new_branches;
}

void Setting::strip_metadata()
{
  for (auto &q : branches)
    q.strip_metadata();
  metadata_ = metadata_.stripped();
}

Setting Setting::text(std::string sid, std::string val)
{
  Setting ret(SettingMeta(sid, SettingType::text));
  ret.value_text = val;
  return ret;
}

Setting Setting::stem(std::string sid)
{
  return Setting(SettingMeta(sid, SettingType::stem));
}

void Setting::set_text(std::string val)
{
  value_text = val;
}

std::string Setting::get_text() const
{
  return value_text;
}

// Numerics

Setting Setting::floating(std::string sid, floating_t val)
{
  Setting ret(SettingMeta(sid, SettingType::floating));
  ret.value_dbl = val;
  return ret;
}

Setting Setting::precise(std::string sid, precise_t val)
{
  Setting ret(SettingMeta(sid, SettingType::precise));
  ret.value_precise = val;
  return ret;
}

Setting Setting::boolean(std::string sid, bool val)
{
  Setting ret(SettingMeta(sid, SettingType::boolean));
  ret.value_int = val;
  return ret;
}

Setting Setting::integer(std::string sid, integer_t val)
{
  Setting ret(SettingMeta(sid, SettingType::integer));
  ret.value_int = val;
  return ret;
}

Setting Setting::indicator(std::string sid, integer_t val)
{
  Setting ret(SettingMeta(sid, SettingType::indicator));
  ret.value_int = val;
  return ret;
}

bool Setting::numeric() const
{
  return metadata_.is_numeric();
}

double Setting::get_number() const
{
  if (is(SettingType::integer)
      || is(SettingType::binary)
      || is(SettingType::menu)
      || is(SettingType::indicator))
    return static_cast<double>(value_int);
  else if (is(SettingType::precise))
    return to_double(value_precise);
  else if (is(SettingType::floating))
    return value_dbl;
  return std::numeric_limits<double>::quiet_NaN();
}

void Setting::set_number(double val)
{
  if (is(SettingType::integer))
    value_int = val;
  else if (is(SettingType::floating))
    value_dbl = val;
  else if (is(SettingType::precise))
    value_precise = val;
  enforce_limits();
}

void Setting::set_precise(PreciseFloat pf)
{
  value_precise = pf;
}

PreciseFloat Setting::precise() const
{
  return value_precise;
}

void Setting::set_int(integer_t v)
{
  value_int = v;
}

integer_t Setting::get_int() const
{
  return value_int;
}

void Setting::select(integer_t v)
{
  value_int = v;
}

integer_t Setting::selection() const
{
  return value_int;
}

bool Setting::get_bool() const
{
  return (0 != value_int);
}

void Setting::set_bool(bool b)
{
  value_int = b;
}

bool Setting::triggered() const
{
  return (0 != value_int);
}

void Setting::trigger()
{
  value_int = 1;
}

void Setting::reset()
{
  value_int = 0;
}

// prefix
Setting& Setting::operator++()
{
  if (is(SettingType::integer))
  {
    value_int += metadata_.step<integer_t>();
    if (value_int > metadata_.max<integer_t>())
      value_int = metadata_.max<integer_t>();
  }
  else if (is(SettingType::floating))
  {
    value_dbl += metadata_.step<floating_t>();
    if (value_dbl > metadata_.max<floating_t>())
      value_dbl = metadata_.max<floating_t>();
  }
  else if (is(SettingType::precise))
  {
    value_precise += metadata_.step<precise_t>();
    if (value_precise > metadata_.max<precise_t>())
      value_precise = metadata_.max<precise_t>();
  }
  return *this;
}

Setting& Setting::operator--()
{
  if (is(SettingType::integer))
  {
    value_int -= metadata_.step<integer_t>();
    if (value_int < metadata_.min<integer_t>())
      value_int = metadata_.min<integer_t>();
  }
  else if (is(SettingType::floating))
  {
    value_dbl -= metadata_.step<floating_t>();
    if (value_dbl < metadata_.min<floating_t>())
      value_dbl = metadata_.min<floating_t>();
  }
  else if (is(SettingType::precise))
  {
    value_precise -= metadata_.step<precise_t>();
    if (value_precise < metadata_.min<precise_t>())
      value_precise = metadata_.min<precise_t>();
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

// lexical

std::string Setting::val_to_string() const
{
  std::stringstream ss;
  if (is(SettingType::integer))
    ss << std::to_string(value_int);
  else if (is(SettingType::floating))
    ss << std::setprecision(std::numeric_limits<floating_t>::max_digits10)
       << value_dbl;
  else if (is(SettingType::precise))
    ss << std::setprecision(std::numeric_limits<precise_t>::max_digits10)
       << value_precise;
  else if (is(SettingType::pattern))
    ss << value_pattern.debug();
  else if (is(SettingType::duration))
    ss << very_simple(value_duration);
  else if (is(SettingType::time))
    ss << to_iso_extended(value_time);
  else if (is(SettingType::boolean))
  {
    if (value_int != 0)
      ss << "True";
    else
      ss << "False";
  }
  else if (is(SettingType::text) ||
           is(SettingType::stem))
    ss << value_text;
  else if (is(SettingType::binary))
  {
    auto size = metadata_.get_num<size_t>("bits", 16);
    if (size > 32)
      ss << "0x" << itohex64(value_int);
    else if (size > 16)
      ss << "0x" << itohex32(value_int);
    else
      ss << "0x" << itohex16(value_int);
  }
  else if (is(SettingType::menu) || is(SettingType::indicator))
  {
    ss << "[" << std::to_string(value_int) << "] "
       << metadata_.enum_name(value_int);
  }

  return ss.str();
}

std::string Setting::debug(std::string prepend, bool verbose) const
{
  std::string ret;
  if (!id().empty())
    ret = col(BLUE, NONE, BRIGHT) + id() + col();
  if (!indices_.empty())
    ret += col(RED) + indices_to_string(true) + col();
  if (!ret.empty())
    ret += "=";
  ret += col(GREEN, NONE, BRIGHT) + val_to_string() + col();
  ret += " " + metadata_.debug(prepend, verbose);
  ret += "\n";
  if (!branches.empty())
    for (size_t i = 0; i < branches.size(); ++i)
    {
      if ((i+1) == branches.size())
        ret += prepend + k_branch_end +
            branches.get(i).debug(prepend + "  ", verbose);
      else
        ret += prepend + k_branch_mid +
            branches.get(i).debug(prepend + k_branch_pre, verbose);
    }
  return ret;
}

//json

json Setting::val_to_json() const
{
  if (is(SettingType::boolean))
    return json(bool(value_int));
  else if (is(SettingType::integer) ||
           is(SettingType::menu) ||
           is(SettingType::binary) ||
           is(SettingType::indicator) )
    return json(value_int);
  else if (is(SettingType::pattern))
    return json(value_pattern);
  else if (is(SettingType::floating))
    return json(value_dbl);
  else if (is(SettingType::precise))
    return json(value_precise);
  else if (is(SettingType::duration))
    return to_simple(value_duration);
  else
    return json(val_to_string());
}

void Setting::val_from_json(const json &j)
{
  if (is(SettingType::boolean))
  {
    if (j.is_number())
      value_int = j.get<integer_t>();
    else if (j.is_boolean())
      value_int = j.get<bool>();
  }
  else if (is(SettingType::integer) ||
           is(SettingType::menu) ||
           is(SettingType::binary) ||
           is(SettingType::indicator) )
    value_int = j.get<integer_t>();
  else if (is(SettingType::pattern))
    value_pattern = j;
  else if (is(SettingType::floating))
    value_dbl = j.get<floating_t>();
  else if (is(SettingType::precise))
    value_precise = j;
  else if (is(SettingType::time))
    value_time = from_iso_extended(j.get<std::string>());
  else if (is(SettingType::duration))
    value_duration = duration_from_string(j.get<std::string>());
  else if (is(SettingType::text))
    value_text = j.get<std::string>();
}


void to_json(json& j, const Setting &s)
{
  j["id"] = s.id();
  j["type"] = to_string(s.metadata_.type());

  if (s.metadata_.meaningful())
    j["metadata"] = s.metadata_;

  if (!s.indices_.empty())
    j["indices"] = s.indices_;

  if (s.is(SettingType::stem))
  {
    if (!s.branches.empty())
      j["branches"] = s.branches;
    if (!s.value_text.empty())
      j["reference"] = s.value_text;
  }
  else
    j["value"] = s.val_to_json();
}

void from_json(const json& j, Setting &s)
{
  if (!j.count("id") || !j.count("type"))
    return;

  s.metadata_ = SettingMeta(j["id"], from_string(j["type"]));

  if (j.count("metadata"))
    s.metadata_ = j["metadata"];

  if (j.count("indices"))
    for (auto it : j["indices"])
      s.indices_.insert(it.get<int32_t>());

  if (s.is(SettingType::stem))
  {
    if (j.count("reference"))
      s.value_text = j["reference"];
    if (j.count("branches"))
      s.branches = j["branches"];
  } else
    s.val_from_json(j["value"]);
}


}


