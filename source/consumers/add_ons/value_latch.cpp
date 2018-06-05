#include "value_latch.h"

namespace DAQuiri {

void ValueLatch::settings(const Setting& s)
{
  auto name = s.find(Setting("name")).get_text();
  if (name != name_)
  {
    idx_ = -1;
    name_ = name;
  }
  downsample_ = static_cast<uint16_t>(s.find(Setting("downsample")).get_int());
}

Setting ValueLatch::settings(int32_t index, std::string override_name) const
{
  auto mret = SettingMeta("value_latch", SettingType::stem,
                          (override_name.empty() ? "Value" : override_name));
  auto ret = Setting(mret);
  ret.set_indices({index});

  SettingMeta mname("name", SettingType::text, "Value ID");
  mname.set_flag("event_value");
  Setting name(mname);
  name.set_text(name_);
  name.set_indices({index});
  ret.branches.add(name);

  SettingMeta mds("downsample", SettingType::integer, "Downsample by");
  mds.set_val("units", "bits");
  mds.set_flag("preset");
  mds.set_val("min", 0);
  mds.set_val("max", 31);
  Setting ds(mds);
  ds.set_int(downsample_);
  ds.set_indices({index});
  ret.branches.add(ds);

  return ret;
}

void ValueLatch::configure(const Spill& spill)
{
  if (has_declared_value(spill))
    idx_ = static_cast<int32_t>(spill.event_model.name_to_val.at(name_));
  else
    idx_ = -1;
}

}
