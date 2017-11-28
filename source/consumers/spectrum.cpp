#include "spectrum.h"

#include "custom_logger.h"

Spectrum::Spectrum()
  : Consumer()
{
  Setting base_options = metadata_.attributes();

  SettingMeta name("name", SettingType::text);
  name.set_val("description", "Short label");
  base_options.branches.add(name);

  SettingMeta vis("visible", SettingType::boolean);
  vis.set_val("description", "Plot visible");
  base_options.branches.add(vis);

  SettingMeta sca("preferred_scale", SettingType::menu, "Initial plotting scale for counts");
  sca.set_enum(0, "Linear");
  sca.set_enum(1, "Logarithmic");
  base_options.branches.add(sca);

  SettingMeta totalcount("total_count", SettingType::precise, "Total count");
  totalcount.set_flag("readonly");
  totalcount.set_val("min", 0);
  base_options.branches.add(totalcount);

  SettingMeta start_time("start_time", SettingType::time);
  start_time.set_val("description", "Start time");
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

  metadata_.overwrite_all_attributes(base_options);
}

bool Spectrum::_initialize()
{
  Consumer::_initialize();

  clear_periodically_ = metadata_.get_attribute("clear_periodically").triggered();
  clear_at_ = metadata_.get_attribute("clear_at").get_number() * 0.001;
  return false;
}

Status Spectrum::extract(const Spill& spill)
{
  Status ret;
  ret.type = spill.type;
  ret.time = spill.time;
  ret.tb = spill.event_model.timebase;
  Setting native_time = spill.state.find(Setting("native_time"));
  if (native_time)
    ret.stats["native_time"] = native_time;
  auto live_time = spill.state.find(Setting("live_time"));
  if (live_time)
    ret.stats["live_time"] = live_time;
  return ret;
}

PreciseFloat Spectrum::calc_recent_rate(const Spill &spill)
{
  if (recent_end_.time.is_not_a_date_time())
    recent_end_ = spill;

  recent_start_ = recent_end_;
  recent_end_ = spill;

//  DBG << recent_end_.stats()["native_time"]
//      <<  " - " << recent_start_.stats()["native_time"];

  Setting rate = metadata_.get_attribute("instant_rate");
  auto diff = recent_end_.state.find(Setting("native_time")).get_number()
      - recent_start_.state.find(Setting("native_time")).get_number();
  PreciseFloat recent_time = recent_end_.event_model.timebase.to_sec(diff);

  rate.set_number(0);
  if (recent_time > 0)
    rate.set_number(recent_count_ / recent_time);

  metadata_.set_attribute(rate, false);
  recent_count_ = 0;

  return recent_time;
}

PreciseFloat Spectrum::calc_chan_times()
{
  Status start = chan_stats.stats.front();

  PreciseFloat rt {0},lt {0};
  PreciseFloat real {0}, live {0};

  for (auto &q : chan_stats.stats)
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

      lt = rt = q.tb.to_microsec(diff_real);
      if (diff_live)
        lt = q.tb.to_microsec(diff_live);
    }
  }

  if (chan_stats.stats.back().type != StatusType::start)
  {
    real += rt;
    live += lt;
    //        DBG << "<Spectrum> \"" << metadata_.name << "\" RT + "
    //               << rt << " = " << real << "   LT + " << lt << " = " << live;
  }
  chan_stats.real = boost::posix_time::microseconds(real);
  chan_stats.live = boost::posix_time::microseconds(live);

  return real;
}


void Spectrum::_push_stats_post(const Spill &spill)
{
  if (!this->_accept_spill(spill))
    return;

//  DBG << "new native time = " << status.stats()["native_time"];

//  bool chan_new = (chan_stats.count(spill.channel()) == 0);
  bool new_start = (spill.type == StatusType::start);

  if (
//      !chan_new &&
      new_start &&
      (chan_stats.stats.back().type == StatusType::running))
    chan_stats.stats.back().type = StatusType::stop;

  recent_total_time_ += calc_recent_rate(spill);
//  DBG << "Recent total time " << recent_total_time_;

  if (
//      !chan_new &&
      (chan_stats.stats.back().type == StatusType::running))
    chan_stats.stats.pop_back();

  chan_stats.stats.push_back(extract(spill));

  if (
//      !chan_new &&
      true
      )
  {
    PreciseFloat real = calc_chan_times();

    Setting live_time = metadata_.get_attribute("live_time");
    Setting real_time = metadata_.get_attribute("real_time");

    live_time.set_duration(boost::posix_time::microseconds(real));
    real_time.set_duration(boost::posix_time::microseconds(real));
    if (chan_stats.real.total_milliseconds() < real_time.duration().total_milliseconds())
      real_time.set_duration(chan_stats.real);

    if (chan_stats.live.total_milliseconds() < live_time.duration().total_milliseconds())
      live_time.set_duration(chan_stats.live);

    metadata_.set_attribute(live_time, false);
    metadata_.set_attribute(real_time, false);

    //      DBG << "<Spectrum> \"" << metadata_.name << "\"  ********* "
    //             << "RT = " << to_simple_string(metadata_.real_time)
    //             << "   LT = " << to_simple_string(metadata_.live_time) ;

  }

  metadata_.set_attribute(Setting::precise("total_count", total_count_), false);

  if (!new_start && clear_periodically_ && data_ &&
      (clear_at_ < recent_total_time_))
  {
    recent_total_time_ -= clear_at_;
    clear_next_spill_ = true;
  }

  this->_recalc_axes();
}

void Spectrum::_push_stats_pre(const Spill &spill)
{
//  if (!this->channel_relevant(status.channel()))
//    return;

  if (metadata_.get_attribute("start_time").time().is_not_a_date_time())
    metadata_.set_attribute(Setting("start_time", spill.time), false);

  if (clear_next_spill_)
  {
    data_->clear();
    total_count_ = 0;
    recent_count_ = 0;
    clear_next_spill_ = false;
  }
}

void Spectrum::_flush()
{
  metadata_.set_attribute(Setting::precise("total_count", total_count_), false);
}
