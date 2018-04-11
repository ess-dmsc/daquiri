#include "spectrum.h"

#include "custom_logger.h"

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

  SettingMeta clear_p("clear_periodically", SettingType::boolean,
                       "Clear periodically");
  clear_p.set_flag("preset");
  base_options.branches.add(clear_p);

  SettingMeta clear_at("clear_at", SettingType::floating,
                       "Clear at real-time threshold");
  clear_at.set_val("min", 0);
  clear_at.set_val("units", "ms");
  clear_at.set_flag("preset");
  base_options.branches.add(clear_at);

  SettingMeta cusing("clear_using", SettingType::menu, "Clear time reference");
  cusing.set_enum(0, "Producer wall clock (receipt of data)");
  cusing.set_enum(1, "Consumer wall clock (time of binning)");
  cusing.set_enum(2, "native_time provided by DAQ");
  base_options.branches.add(cusing);

  metadata_.overwrite_all_attributes(base_options);
}

void Spectrum::_apply_attributes()
{
  Consumer::_apply_attributes();

  clear_periodically_ = metadata_.get_attribute("clear_periodically").triggered();
  clear_at_ = metadata_.get_attribute("clear_at").get_number() * 0.001;
  clear_reference_timer_ = metadata_.get_attribute("clear_using").selection();
}

bool Spectrum::_accept_spill(const Spill& spill)
{
  return (Consumer::_accept_spill(spill)
          && (spill.type != StatusType::daq_status));
}

Status Spectrum::extract(const Spill& spill)
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

void Spectrum::calc_recent_rate(const Spill& spill)
{
//  DBG << "Processing spill " << spill.stream_id << " "
//      << type_to_str(spill.type) << "  " << spill.time;

  if (recent_end_.producer_time.is_not_a_date_time())
    recent_end_ = extract(spill);
  recent_start_ = recent_end_;
  recent_end_ = extract(spill);

  auto diff = recent_end_.stats["native_time"].get_number()
      - recent_start_.stats["native_time"].get_number();
  PreciseFloat recent_native_time = recent_end_.timebase.to_sec(diff);

  boost::posix_time::time_duration dif2
      = recent_end_.producer_time - recent_start_.producer_time;
  PreciseFloat recent_producer_wall_time = dif2.total_milliseconds() * 0.001;

  boost::posix_time::time_duration dif3
      = recent_end_.consumer_time - recent_start_.consumer_time;
  PreciseFloat recent_consumer_wall_time = dif3.total_milliseconds() * 0.001;

//  DBG << recent_end_.producer_time << " - "
//      << recent_start_.producer_time
//      << " = " << recent_producer_wall_time;

  double instrate = 0;
  if (recent_native_time > 0)
    instrate = recent_count_ / recent_native_time;
  metadata_.set_attribute(Setting::floating("instant_rate", instrate), false);

  recent_count_ = 0;

  recent_native_time_ += recent_native_time;
  recent_producer_wall_time_ += recent_producer_wall_time;
  recent_consumer_wall_time_ += recent_consumer_wall_time;

//  DBG << "Recent times " << recent_native_time_
//      << " " << recent_producer_wall_time_
//      << " " << recent_consumer_wall_time_;
}

void Spectrum::calc_cumulative()
{
  Status start = stats_.front();

  PreciseFloat rt {0},lt {0};
  PreciseFloat real {0}, live {0};

  for (auto &q : stats_)
  {
    if (q.type == StatusType::start)
    {
      real += rt;
      live += lt;
      start = q;
    }
    else
    {
      PreciseFloat diff_real{0}, diff_live{0};
      if (q.stats.count("native_time") && start.stats.count("native_time"))
        diff_real = q.stats.at("native_time").get_number()
            - start.stats.at("native_time").get_number();
      if (q.stats.count("live_time") && start.stats.count("live_time"))
        diff_live = q.stats.at("live_time").get_number()
            - start.stats.at("live_time").get_number();

      lt = rt = q.timebase.to_microsec(diff_real);
      if (diff_live)
        lt = q.timebase.to_microsec(diff_live);
    }
  }

  if (stats_.back().type != StatusType::start)
  {
    real += rt;
    live += lt;
    //        DBG << "<Spectrum> \"" << metadata_.name << "\" RT + "
    //               << rt << " = " << real << "   LT + " << lt << " = " << live;
  }
  real_time_ = boost::posix_time::microseconds(real);
  live_time_ = boost::posix_time::microseconds(live);
}


void Spectrum::_push_stats_post(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  calc_recent_rate(spill);

  if (stats_.size() &&
      (stats_.back().type == StatusType::running))
    stats_.pop_back();

  stats_.push_back(extract(spill));

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

  double* recent_time = &recent_producer_wall_time_;
  if (clear_reference_timer_ == 1)
    recent_time = &recent_consumer_wall_time_;
  else if (clear_reference_timer_ == 2)
    recent_time = &recent_native_time_;

  if ((spill.type != StatusType::start) &&
      clear_periodically_ && data_ &&
      ((*recent_time) > clear_at_))
  {
//    DBG << "Clear triggered for " << metadata_.get_attribute("name").get_text()
//        << " using " << clear_reference_timer_ << " satisfied "
//        << (*recent_time) << " > " << clear_at_;

    (*recent_time) -= clear_at_;
    clear_next_spill_ = true;
  }

  this->_recalc_axes();
}

void Spectrum::_push_stats_pre(const Spill &spill)
{
  if (!this->_accept_spill(spill))
    return;

  if (metadata_.get_attribute("start_time").time().is_not_a_date_time())
    metadata_.set_attribute(Setting("start_time", spill.time), false);

  if (clear_next_spill_)
  {
    data_->clear();
    recent_count_ = 0;
    clear_next_spill_ = false;
  }
}

void Spectrum::_flush()
{
  metadata_.set_attribute(Setting::precise("total_count", data_->total_count()), false);
}
