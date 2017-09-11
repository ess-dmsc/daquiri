#include "spectrum.h"

Spectrum::Spectrum()
  : Consumer()
{
  Setting base_options = metadata_.attributes();

  SettingMeta sca("preferred_scale", SettingType::menu);
  sca.set_flag("preset");
  sca.set_enum(0, "Linear");
  sca.set_enum(1, "Logarithmic");
  base_options.branches.add(sca);

  SettingMeta totalcount("total_count", SettingType::precise, "Total count");
  totalcount.set_val("min", 0);
  totalcount.set_flag("readonly");
  base_options.branches.add(totalcount);

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

  SettingMeta clear_at("clear_at", SettingType::integer,
                       "Clear real-time threshold");
  clear_at.set_val("min", 0);
  clear_at.set_val("units", "s");
  clear_at.set_flag("preset");
  base_options.branches.add(clear_at);

  metadata_.overwrite_all_attributes(base_options);
}

bool Spectrum::_initialize()
{
  clear_periodically_ = metadata_.get_attribute("clear_periodically").triggered();
  clear_at_ = metadata_.get_attribute("clear_at").get_number();
  return false;
}

bool Spectrum::value_relevant(int16_t channel, const std::vector<int>& idx)
{
  return (channel < static_cast<int16_t>(idx.size()))
      && (idx.at(channel) >= 0);
}

void Spectrum::_push_stats(const Status& status)
{
  if (!this->channel_relevant(status.channel()))
    return;

  bool chan_new = (stats_list_.count(status.channel()) == 0);
  bool new_start = (status.type() == StatusType::start);

  if (metadata_.get_attribute("start_time").time().is_not_a_date_time())
    metadata_.set_attribute(Setting("start_time", status.time()), false);

  if (!chan_new
      && new_start
      && (stats_list_[status.channel()].back().type() == StatusType::running))
    stats_list_[status.channel()].back().set_type(StatusType::stop);

  if (recent_end_.time().is_not_a_date_time())
    recent_start_ = status;
  else
    recent_start_ = recent_end_;

  recent_end_ = status;

  Setting rate = metadata_.get_attribute("instant_rate");
  rate.set_number(0);
  double recent_time =
      (recent_end_.time() - recent_start_.time()).total_milliseconds() * 0.001;
  if (recent_time > 0)
    rate.set_number(recent_count_ / recent_time);

  metadata_.set_attribute(rate, false);

  recent_count_ = 0;

  if (!chan_new && (stats_list_[status.channel()].back().type() == StatusType::running))
    stats_list_[status.channel()].pop_back();

  stats_list_[status.channel()].push_back(status);

  if (!chan_new)
  {
    Status start = stats_list_[status.channel()].front();

    boost::posix_time::time_duration rt ,lt;
    boost::posix_time::time_duration real ,live;

    for (auto &q : stats_list_[status.channel()])
    {
      if (q.type() == StatusType::start)
      {
        real += rt;
        live += lt;
        start = q;
      }
      else
      {
        lt = rt = q.time() - start.time();
        PreciseFloat diff_native{0}, diff_live{0};
        if (q.stats().count("native_time") && start.stats().count("native_time"))
          diff_native = q.stats().at("native_time") - start.stats().at("native_time");
        if (q.stats().count("live_time") && start.stats().count("live_time"))
          diff_live = q.stats().at("live_time") - start.stats().at("live_time");

        PreciseFloat scale_factor = 1;
        if (diff_native > 0)
          scale_factor = rt.total_microseconds() / diff_native;
        if (diff_live)
        {
          PreciseFloat scaled_live = diff_live * scale_factor;
          lt = boost::posix_time::microseconds(static_cast<double>(scaled_live));
        }
      }
    }

    if (stats_list_[status.channel()].back().type() != StatusType::start)
    {
      real += rt;
      live += lt;
      //        DBG << "<Spectrum> \"" << metadata_.name << "\" RT + "
      //               << rt << " = " << real << "   LT + " << lt << " = " << live;
    }
    real_times_[status.channel()] = real;
    live_times_[status.channel()] = live;

    Setting live_time = metadata_.get_attribute("live_time");
    Setting real_time = metadata_.get_attribute("real_time");

    live_time.set_duration(real);
    real_time.set_duration(real);
    for (auto &q : real_times_)
      if (q.second.total_milliseconds() < real_time.duration().total_milliseconds())
        real_time.set_duration(q.second);

    for (auto &q : live_times_)
      if (q.second.total_milliseconds() < live_time.duration().total_milliseconds())
        live_time.set_duration(q.second);

    metadata_.set_attribute(live_time, false);
    metadata_.set_attribute(real_time, false);

    //      DBG << "<Spectrum> \"" << metadata_.name << "\"  ********* "
    //             << "RT = " << to_simple_string(metadata_.real_time)
    //             << "   LT = " << to_simple_string(metadata_.live_time) ;

  }

  metadata_.set_attribute(Setting::precise("total_count", total_count_), false);

  recent_total_time_ += recent_time;
  if (!new_start && clear_periodically_ && data_ &&
      (clear_at_ < recent_total_time_))
  {
    recent_total_time_ -= clear_at_;
    clear_next_spill_ = true;
  }

  this->_recalc_axes();
}

void Spectrum::_push_spill(const Spill& spill)
{
  if (clear_next_spill_)
  {
    bool relevant {false};
    for (auto &q : spill.stats)
      if (this->channel_relevant(q.first))
      {
        relevant = true;
        break;
      }

    if (relevant)
    {
      data_->clear();
      total_count_ = 0;
      recent_count_ = 0;
      clear_next_spill_ = false;
    }
  }

  Consumer::_push_spill(spill);
}

void Spectrum::_flush()
{
  metadata_.set_attribute(Setting::precise("total_count", total_count_), false);
}
