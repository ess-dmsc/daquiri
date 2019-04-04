#include <consumers/add_ons/periodic_trigger.h>

#include <core/util/logger.h>

namespace DAQuiri {

void PeriodicTrigger::settings(const Setting& s)
{
  enabled = s.find(Setting("periodic_trigger/enabled")).get_bool();
  clock_type =
      static_cast<ClockType>(s.find(Setting("periodic_trigger/clock")).selection());
  timeout = s.find(Setting("periodic_trigger/time_out")).duration();
}

Setting PeriodicTrigger::settings(int32_t index, std::string override_name) const
{
  auto mret = SettingMeta("periodic_trigger", SettingType::stem,
                          (override_name.empty() ? "Periodic trigger" : override_name));
  auto ret = Setting(mret);
  if (index >= 0)
    ret.set_indices({index});

  SettingMeta enm("periodic_trigger/enabled", SettingType::boolean, "Enabled");
  Setting en(enm);
  en.set_bool(enabled);
  if (index >= 0)
    en.set_indices({index});
  ret.branches.add(en);

  SettingMeta cusing("periodic_trigger/clock", SettingType::menu, "Clock");
  cusing.set_enum(ClockType::ProducerWallClock,
                  "Producer wall clock (receipt of data)");
  cusing.set_enum(ClockType::ConsumerWallClock,
                  "Consumer wall clock (time of binning)");
  cusing.set_enum(ClockType::NativeTime,
                  "native_time provided by DAQ");
  cusing.set_flag("preset");
  Setting ccusing(cusing);
  ccusing.select(clock_type);
  if (index >= 0)
    ccusing.set_indices({index});
  ret.branches.add(ccusing);

  SettingMeta clear_at("periodic_trigger/time_out", SettingType::duration,
                       "Time-out");
  clear_at.set_flag("preset");
  Setting cat(clear_at);
  cat.set_duration(timeout);
  if (index >= 0)
    cat.set_indices({index});
  ret.branches.add(cat);

  return ret;
}

void PeriodicTrigger::update(const Status& current)
{
  if (!previous_.valid) {
    previous_ = current;
    return;
  }

  if (clock_type == ClockType::NativeTime)
  {
    auto recent_native_time = Status::calc_diff(previous_, current, "native_time");
    if (recent_native_time != hr_duration_t())
      recent_time_ += recent_native_time;
  }
  else if ((clock_type == ClockType::ProducerWallClock) &&
      (current.producer_time != hr_time_t()) && previous_.valid)
  {
    recent_time_ += (current.producer_time - previous_.producer_time);
  }
  else if ((clock_type == ClockType::ConsumerWallClock) &&
          (current.consumer_time != hr_time_t()) && previous_.valid)
  {
    recent_time_ += (current.consumer_time - previous_.consumer_time);
  }

  previous_ = current;
  eval_trigger();
}

void PeriodicTrigger::eval_trigger()
{
  if (enabled && (recent_time_ >= timeout))
  {
    recent_time_ -= timeout;
    triggered = true;
  }
}

}