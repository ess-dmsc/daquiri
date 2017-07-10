#include "spectrum.h"
#include "custom_logger.h"

Spectrum::Spectrum()
  : Consumer()
{
  Setting attributes = metadata_.attributes();

  SettingMeta totalcoinc("total_count", SettingType::precise);
  totalcoinc.set_val("description", "Total count");
  totalcoinc.set_flag("readonly");
  Setting totc(totalcoinc);
  totc.set_number(0);
  attributes.branches.add(totc);

  SettingMeta live_time("live_time", SettingType::duration);
  live_time.set_flag("readonly");
  live_time.set_val("description", "Live time");
  attributes.branches.add(live_time);

  SettingMeta real_time("real_time", SettingType::duration);
  real_time.set_flag("readonly");
  real_time.set_val("description", "Real time");
  attributes.branches.add(real_time);

  SettingMeta inst_rate("instant_rate", SettingType::floating);
  inst_rate.set_flag("readonly");
  inst_rate.set_val("description", "Instant count rate");
  inst_rate.set_val("units", "cps");
  Setting inst(inst_rate);
  inst.set_number(0);
  attributes.branches.add(inst);

  metadata_.overwrite_all_attributes(attributes);
}

void Spectrum::_init_from_file(std::string name)
{
  metadata_.set_attribute(Setting::precise("total_count", total_count_), false);
  Consumer::_init_from_file(name);
}

void Spectrum::_push_stats(const Status& newBlock)
{
  //private; no lock required

  if (!this->channel_relevant(newBlock.channel()))
    return;

  //DBG << "Spectrum " << metadata_.name << " received update for chan " << newBlock.channel;
  bool chan_new = (stats_list_.count(newBlock.channel()) == 0);
  bool new_start = (newBlock.type() == StatusType::start);

  if (metadata_.get_attribute("start_time").time().is_not_a_date_time())
    metadata_.set_attribute(Setting("start_time", newBlock.time()), false);

  if (!chan_new
      && new_start
      && (stats_list_[newBlock.channel()].back().type() == StatusType::running))
    stats_list_[newBlock.channel()].back().set_type(StatusType::stop);

  if (recent_end_.time().is_not_a_date_time())
    recent_start_ = newBlock;
  else
    recent_start_ = recent_end_;

  recent_end_ = newBlock;

  Setting rate = metadata_.get_attribute("instant_rate");
  rate.set_number(0);
  double recent_time =
      (recent_end_.time() - recent_start_.time()).total_milliseconds() * 0.001;
  if (recent_time > 0)
    rate.set_number(recent_count_ / recent_time);
  metadata_.set_attribute(rate, false);

  recent_count_ = 0;

  if (!chan_new && (stats_list_[newBlock.channel()].back().type() == StatusType::running))
    stats_list_[newBlock.channel()].pop_back();

  stats_list_[newBlock.channel()].push_back(newBlock);

  if (!chan_new)
  {
    Status start = stats_list_[newBlock.channel()].front();

    boost::posix_time::time_duration rt ,lt;
    boost::posix_time::time_duration real ,live;

    for (auto &q : stats_list_[newBlock.channel()])
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

    if (stats_list_[newBlock.channel()].back().type() != StatusType::start) {
      real += rt;
      live += lt;
      //        DBG << "<Spectrum> \"" << metadata_.name << "\" RT + "
      //               << rt << " = " << real << "   LT + " << lt << " = " << live;
    }
    real_times_[newBlock.channel()] = real;
    live_times_[newBlock.channel()] = live;

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

  metadata_.set_attribute(Setting::precise("total_hits", total_count_), false);
}


void Spectrum::_flush()
{
  metadata_.set_attribute(Setting::precise("total_hits", total_count_), false);
}

void Spectrum::_set_detectors(const std::vector<Detector>& dets)
{
  //private; no lock required
  //  DBG << "<Spectrum> _set_detectors";

  metadata_.detectors.clear();

  //FIX THIS!!!!

  //metadata_.detectors.resize(metadata_.dimensions(), Detector());
  this->_recalc_axes();
}

void Spectrum::_recalc_axes()
{
  //private; no lock required

  axes_.resize(metadata_.dimensions());
  if (axes_.size() != metadata_.detectors.size())
    return;

  for (size_t i=0; i < metadata_.detectors.size(); ++i)
  {
//    Calibration this_calib = metadata_.detectors[i].best_calib(bits_);
//    uint32_t res = pow(2,bits_);
//    axes_[i].resize(res, 0.0);
//    for (uint32_t j=0; j<res; j++)
//      axes_[i][j] = this_calib.transform(j, bits_);
  }
}
