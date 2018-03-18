#include "value_filter.h"

using namespace DAQuiri;

void ValueFilter::settings(const Setting& s)
{
  auto name = s.find(Setting("name")).get_text();
  if (name != name_)
  {
    idx_ = -1;
    name_ = name;
  }

  enabled_ = s.find(Setting("enabled")).triggered();
  min_ = s.find(Setting("min")).get_number();
  max_ = s.find(Setting("max")).get_number();
}

Setting ValueFilter::settings(int32_t index) const
{
  auto ret = Setting::stem("filter");

  SettingMeta en("enabled", SettingType::boolean, "Enabled");
  Setting enabled(en);
  enabled.set_bool(enabled_);
  enabled.set_indices({index});
  ret.branches.add(enabled);

  SettingMeta mname("name", SettingType::boolean, "Value ID");
  mname.set_flag("event_value");
  Setting name(mname);
  name.set_text(name_);
  name.set_indices({index});
  ret.branches.add(name);

  SettingMeta mmin("min", SettingType::integer, "Minimum value");
  mmin.set_val("min", 0);
  mmin.set_val("max", std::numeric_limits<uint32_t>::max());
  Setting min(mmin);
  min.set_int(min_);
  min.set_indices({index});
  ret.branches.add(min);

  SettingMeta mmax("max", SettingType::integer, "Maximum value");
  mmax.set_val("min", 0);
  mmax.set_val("max", std::numeric_limits<uint32_t>::max());
  Setting max(mmax);
  max.set_int(max_);
  max.set_indices({index});
  ret.branches.add(max);

  return ret;
}

void FilterBlock::settings(const Setting& s)
{
  size_t filter_count = s.find(Setting("filter_count")).get_number();
  filters_.resize(filter_count);

  size_t i = 0;
  for (auto ss : s.branches)
  {
    filters_[i++].settings(ss);
    if (i >= filter_count)
      break;
  }
}

Setting FilterBlock::settings() const
{
  auto ret = Setting::stem("filter");

  SettingMeta fc("filter_count", SettingType::integer, "Number of filters");
  fc.set_val("min", 0);
  fc.set_val("max", 16);
  Setting filter_count(fc);
  filter_count.set_int(filters_.size());
  ret.branches.add(filter_count);

  for (size_t i=0; i < filters_.size(); ++i)
    ret.branches.add(filters_[i].settings(i));

  return ret;
}
