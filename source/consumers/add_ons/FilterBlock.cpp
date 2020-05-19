#include <consumers/add_ons/FilterBlock.h>

namespace DAQuiri {

void FilterBlock::settings(const Setting& s)
{
  size_t filter_count = 0;
  auto newsize = s.find(Setting("filter_count"));
  if (newsize)
    filter_count = newsize.get_int();
  filters_.resize(filter_count);

  size_t i = 0;
  for (auto ss : s.branches)
  {
    if (ss.id() == "filter_count")
      continue;
    filters_[i++].settings(ss);
    if (i >= filter_count)
      break;
  }

}

Setting FilterBlock::settings() const
{
  auto ret = Setting::stem("filters");

  SettingMeta fc("filter_count", SettingType::integer, "Number of filters");
  fc.set_val("min", 0);
  fc.set_val("max", 16);
  Setting filter_count(fc);
  filter_count.set_int(filters_.size());
  ret.branches.add(filter_count);

  for (size_t i = 0; i < filters_.size(); ++i)
    ret.branches.add_a(filters_[i].settings(i));

  return ret;
}

void FilterBlock::configure(const Spill& spill)
{
  valid = false;
  for (auto& f : filters_)
  {
    if (f.enabled_)
      f.configure(spill);
    if (f.valid())
      valid = true;
  }
}

}
