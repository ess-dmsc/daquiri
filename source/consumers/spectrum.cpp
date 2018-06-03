#include "spectrum.h"

#include "custom_logger.h"

namespace DAQuiri {

Status Status::extract(const Spill& spill)
{
  Status ret;
  ret.type = spill.type;
  ret.producer_time = spill.time;
  ret.consumer_time = boost::posix_time::microsec_clock::universal_time();
  ret.timebase = spill.event_model.timebase;
  Setting native_time = spill.state.find(Setting("native_time"));
  if (native_time)
    ret.stats["native_time"] = native_time;
  auto live_time = spill.state.find(Setting("live_time"));
  if (live_time)
    ret.stats["live_time"] = live_time;
  return ret;
}

boost::posix_time::time_duration
Status::calc_diff(const std::vector<Status>& stats, std::string name)
{
  PreciseFloat t{0};
  PreciseFloat ret{0};

  Status start = stats.front();

  for (auto& q : stats)
  {
    if (q.type == StatusType::start)
    {
      ret += t;
      start = q;
    }
    else
    {
      PreciseFloat diff{0};
      if (q.stats.count(name) && start.stats.count(name))
        diff = q.stats.at(name).get_number() - start.stats.at(name).get_number();
      t = q.timebase.to_microsec(diff);
    }
  }

  if (stats.back().type != StatusType::start)
    ret += t;

  return boost::posix_time::microseconds(ret);
}

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



Spectrum::Spectrum()
    : Consumer()
{
  Setting base_options = metadata_.attributes();

  SettingMeta name("name", SettingType::text, "Name");
  base_options.branches.add(name);

  SettingMeta vis("visible", SettingType::boolean, "Visible");
  base_options.branches.add(vis);

  SettingMeta sca("preferred_scale", SettingType::menu, "Plot scale for counts");
  sca.set_enum(0, "Linear");
  sca.set_enum(1, "Logarithmic");
  base_options.branches.add(sca);

  SettingMeta totalcount("total_count", SettingType::precise, "Total count");
  totalcount.set_flag("readonly");
  totalcount.set_val("min", 0);
  base_options.branches.add(totalcount);

  SettingMeta start_time("start_time", SettingType::time, "Start time");
  start_time.set_flag("readonly");
  base_options.branches.add(start_time);

  SettingMeta live_time("live_time", SettingType::duration, "Live time");
  live_time.set_flag("readonly");
  base_options.branches.add(live_time);

  SettingMeta real_time("real_time", SettingType::duration, "Real time");
  real_time.set_flag("readonly");
  base_options.branches.add(real_time);

  SettingMeta inst_rate("instant_rate", SettingType::floating, "Instant count rate");
  inst_rate.set_flag("readonly");
  inst_rate.set_val("min", 0);
  inst_rate.set_val("units", "cps");
  Setting inst(inst_rate);
  inst.set_number(0);
  base_options.branches.add(inst);

  base_options.branches.add(periodic_trigger_.settings(0));

  metadata_.overwrite_all_attributes(base_options);
}

void Spectrum::_apply_attributes()
{
  Consumer::_apply_attributes();

  periodic_trigger_.settings(metadata_.get_attribute("clear_periodically"));
}

bool Spectrum::_accept_spill(const Spill& spill)
{
  return (Consumer::_accept_spill(spill)
      && (spill.type != StatusType::daq_status));
}

void Spectrum::calc_recent_rate(const Spill& spill)
{
//  DBG << "Processing spill " << spill.stream_id << " "
//      << type_to_str(spill.type) << "  " << spill.time;

  if (recent_end_.type == StatusType::daq_status)
    recent_end_ = Status::extract(spill);
  recent_start_ = recent_end_;
  recent_end_ = Status::extract(spill);

  auto recent_native_time = recent_end_.stats["native_time"].get_number()
      - recent_start_.stats["native_time"].get_number();

//  DBG << recent_end_.producer_time << " - "
//      << recent_start_.producer_time
//      << " = " << recent_producer_wall_time;

  double instant_rate = 0;
  PreciseFloat recent_s = recent_end_.timebase.to_sec(recent_native_time);
  if (recent_s > 0)
    instant_rate = double(recent_count_) / to_double(recent_s);
  metadata_.set_attribute(Setting::floating("instant_rate", instant_rate), false);

  recent_count_ = 0;

  periodic_trigger_.update_times(recent_start_, recent_end_);
}

void Spectrum::calc_cumulative()
{
  live_time_ = Status::calc_diff(stats_, "live_time");
  real_time_ = Status::calc_diff(stats_, "native_time");
  if (live_time_.is_not_a_date_time())
    live_time_ = real_time_;
}

void Spectrum::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  if (metadata_.get_attribute("start_time").time().is_not_a_date_time())
    metadata_.set_attribute(Setting("start_time", spill.time), false);

  if (periodic_trigger_.triggered_)
  {
    data_->clear();
    recent_count_ = 0;
    periodic_trigger_.triggered_ = false;
  }
}

void Spectrum::_push_stats_post(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  calc_recent_rate(spill);

  if (stats_.size() &&
      (stats_.back().type == StatusType::running))
    stats_.pop_back();

  stats_.push_back(Status::extract(spill));

  if (stats_.size())
  {
    calc_cumulative();
    metadata_.set_attribute(Setting("live_time", live_time_), false);
    metadata_.set_attribute(Setting("real_time", real_time_), false);

    //      DBG << "<Spectrum> \"" << metadata_.name << "\"  ********* "
    //             << "RT = " << to_simple_string(metadata_.real_time)
    //             << "   LT = " << to_simple_string(metadata_.live_time) ;

  }

  metadata_.set_attribute(Setting::precise("total_count", data_->total_count()), false);

  if ((spill.type != StatusType::start) && data_)
    periodic_trigger_.eval_trigger();

  this->_recalc_axes();
}

void Spectrum::_flush()
{
  metadata_.set_attribute(Setting::precise("total_count", data_->total_count()), false);
}

}