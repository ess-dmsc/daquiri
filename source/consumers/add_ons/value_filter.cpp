#include "value_filter.h"

namespace DAQuiri {

void ValueFilter::settings(const Setting& s)
{
  auto name = s.find(Setting("name")).get_text();
  if (name != name_)
  {
    idx_ = -1;
    name_ = name;
  }

  enabled_ = s.find(Setting("enabled")).triggered();
  min_ = static_cast<uint32_t>(s.find(Setting("min")).get_int());
  max_ = static_cast<uint32_t>(s.find(Setting("max")).get_int());
}

Setting ValueFilter::settings(int32_t index) const
{
  auto ret = Setting::stem("filter");
  ret.set_indices({index});

  SettingMeta en("enabled", SettingType::boolean, "Enabled");
  Setting enabled(en);
  enabled.set_bool(enabled_);
  enabled.set_indices({index});
  ret.branches.add(enabled);

  SettingMeta mname("name", SettingType::text, "Value ID");
  mname.set_flag("event_value");
  Setting name(mname);
  name.set_text(name_);
  name.set_indices({index});
  ret.branches.add(name);

  SettingMeta mmin("min", SettingType::integer, "Minimum value");
  mmin.set_val("min", 0);
  mmin.set_val("max", std::numeric_limits<int32_t>::max());
  Setting min(mmin);
  min.set_int(min_);
  min.set_indices({index});
  ret.branches.add(min);

  SettingMeta mmax("max", SettingType::integer, "Maximum value");
  mmax.set_val("min", 0);
  mmax.set_val("max", std::numeric_limits<int32_t>::max());
  Setting max(mmax);
  max.set_int(max_);
  max.set_indices({index});
  ret.branches.add(max);

  return ret;
}

void ValueFilter::configure(const Spill& spill)
{
  if (spill.event_model.name_to_val.count(name_))
    idx_ = spill.event_model.name_to_val.at(name_);
  else
    idx_ = -1;
}

}
