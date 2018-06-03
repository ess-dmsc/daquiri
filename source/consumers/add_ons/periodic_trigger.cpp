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

void PeriodicTrigger::update_times(const Status& from, const Status& to)
{
  auto recent_native_time = to.stats.at("native_time").get_number()
      - from.stats.at("native_time").get_number();

  recent_native_time_ +=
      boost::posix_time::microseconds(to.timebase.to_microsec(recent_native_time));
  recent_producer_wall_time_ +=
      (to.producer_time - from.producer_time);
  recent_consumer_wall_time_ +=
      (to.consumer_time - from.consumer_time);
}

void PeriodicTrigger::eval_trigger()
{
  boost::posix_time::time_duration* recent_time = &recent_producer_wall_time_;
  if (clear_using_ == ClearReferenceTime::ConsumerWallClock)
    recent_time = &recent_consumer_wall_time_;
  else if (clear_using_ == ClearReferenceTime::NativeTime)
    recent_time = &recent_native_time_;

  if (enabled_ && ((*recent_time) > timeout_))
  {
//    DBG << "Clear triggered for " << metadata_.get_attribute("name").get_text()
//        << " using " << clear_reference_timer_ << " satisfied "
//        << (*recent_time) << " > " << clear_at_;

    (*recent_time) -= timeout_;
    triggered_ = true;
  }
}

}