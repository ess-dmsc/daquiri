#include <consumers/add_ons/value_latch.h>

namespace DAQuiri {

void ValueLatch::settings(const Setting& s)
{
  auto name = s.find(Setting("value_latch/value_id")).get_text();
  if (name != value_id)
  {
    idx = -1;
    value_id = name;
  }
  downsample = static_cast<uint16_t>(s.find(Setting("value_latch/downsample")).get_int());
}

Setting ValueLatch::settings(int32_t index, std::string override_name) const
{
  auto mret = SettingMeta("value_latch", SettingType::stem,
                          (override_name.empty() ? "Value" : override_name));
  auto ret = Setting(mret);
  if (index >= 0)
    ret.set_indices({index});

  SettingMeta mname("value_latch/value_id", SettingType::text, "Value ID");
  mname.set_flag("event_value");
  Setting name(mname);
  name.set_text(value_id);
  if (index >= 0)
    name.set_indices({index});
  ret.branches.add(name);

  SettingMeta mds("value_latch/downsample", SettingType::integer, "Downsample by");
  mds.set_val("units", "bits");
  mds.set_flag("preset");
  mds.set_val("min", 0);
  mds.set_val("max", 31);
  Setting ds(mds);
  ds.set_int(downsample);
  if (index >= 0)
    ds.set_indices({index});
  ret.branches.add(ds);

  return ret;
}

void ValueLatch::configure(const Spill& spill)
{
  if (has_declared_value(spill))
    idx = static_cast<int32_t>(spill.event_model.name_to_val.at(value_id));
  else
    idx = -1;
}

}
