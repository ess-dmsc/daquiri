#include "periodic_trigger.h"

#include "custom_logger.h"

namespace DAQuiri {

void PeriodicTrigger::settings(const Setting& s)
{
  enabled_ = s.find(Setting("enabled")).get_bool();
  clear_using_ =
      static_cast<ClearReferenceTime>(s.find(Setting("clear_using")).selection());
  timeout_ = s.find(Setting("clear_at")).duration();

}

Setting PeriodicTrigger::settings(int32_t index) const
{
  auto ret = Setting::stem("clear_periodically");
  ret.set_indices({index});

  SettingMeta en("enabled", SettingType::boolean, "Enabled");
  Setting enabled(en);
  enabled.set_bool(enabled_);
  enabled.set_indices({index});
  ret.branches.add(enabled);

  SettingMeta cusing("clear_using", SettingType::menu, "Clear time reference");
  cusing.set_enum(ClearReferenceTime::ProducerWallClock,
                  "Producer wall clock (receipt of data)");
  cusing.set_enum(ClearReferenceTime::ConsumerWallClock,
                  "Consumer wall clock (time of binning)");
  cusing.set_enum(ClearReferenceTime::NativeTime,
                  "native_time provided by DAQ");
  cusing.set_flag("preset");
  Setting ccusing(cusing);
  ccusing.select(clear_using_);
  ccusing.set_indices({index});
  ret.branches.add(ccusing);

  SettingMeta clear_at("clear_at", SettingType::duration,
                       "Clear at time-out");
  clear_at.set_flag("preset");
  Setting cat(clear_at);
  cat.set_duration(timeout_);
  cat.set_indices({index});
  ret.branches.add(cat);

  return ret;
}

void PeriodicTrigger::update(const Status& current)
{
  if (!previous_.valid)
    previous_ = current;

  if (clear_using_ == ClearReferenceTime::NativeTime)
  {
    auto recent_native_time = Status::calc_diff(previous_, current, "native_time");
    if (!recent_native_time.is_not_a_date_time())
      recent_time_ += recent_native_time;
  }
  else if ((clear_using_ == ClearReferenceTime::ProducerWallClock) &&
      !current.producer_time.is_not_a_date_time() &&
      !previous_.producer_time.is_not_a_date_time())
  {
    recent_time_ += (current.producer_time - previous_.producer_time);
  }
  else if ((clear_using_ == ClearReferenceTime::ConsumerWallClock) &&
      !current.consumer_time.is_not_a_date_time() &&
      !previous_.consumer_time.is_not_a_date_time())
  {
    recent_time_ += (current.consumer_time - previous_.consumer_time);
  }

  previous_ = current;

  if (current.type != StatusType::start)
    eval_trigger();
}

void PeriodicTrigger::eval_trigger()
{
  if (enabled_ && (recent_time_ >= timeout_))
  {
    recent_time_ -= timeout_;
    triggered_ = true;
  }
}

}