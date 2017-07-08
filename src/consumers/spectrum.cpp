#include "spectrum.h"
#include "custom_logger.h"

Spectrum::Spectrum()
  : Consumer()
  , recent_count_(0)
  , coinc_window_(0)
  , max_delay_(0)
  , bits_(0)
{
  Setting attributes = metadata_.attributes();

  SettingMeta totalct("total_events", SettingType::precise);
  totalct.set_val("description", "Total event count");
  totalct.set_flag("readonly");
  Setting tots(totalct);
  tots.set_number(0);
  attributes.branches.add(tots);

  SettingMeta totalcoinc("total_coinc", SettingType::precise);
  totalcoinc.set_val("description", "Total coincidence count");
  totalcoinc.set_flag("readonly");
  Setting totc(totalcoinc);
  totc.set_number(0);
  attributes.branches.add(totc);

  SettingMeta resm("resolution", SettingType::menu);
  resm.set_flag("preset");
  resm.set_enum(4, "4 bit (16)");
  resm.set_enum(5, "5 bit (32)");
  resm.set_enum(6, "6 bit (64)");
  resm.set_enum(7, "7 bit (128)");
  resm.set_enum(8, "8 bit (256)");
  resm.set_enum(9, "9 bit (512)");
  resm.set_enum(10, "10 bit (1024)");
  resm.set_enum(11, "11 bit (2048)");
  resm.set_enum(12, "12 bit (4096)");
  resm.set_enum(13, "13 bit (8192)");
  resm.set_enum(14, "14 bit (16384)");
  resm.set_enum(15, "15 bit (32768)");
  resm.set_enum(16, "16 bit (65536)");
  Setting res(resm);
  res.set_number(14);
  attributes.branches.add(res);


  Setting det({"detector", SettingType::stem});
  det.set_indices({-1});

  SettingMeta ignore_zero("cutoff_logic", SettingType::integer);
  ignore_zero.set_flag("preset");
  ignore_zero.set_val("description", "Hits rejected below minimum energy (before coincidence logic)");
  ignore_zero.set_val("min", 0);
  det.branches.add(ignore_zero);

  SettingMeta delay("delay_ns", SettingType::floating);
  delay.set_flag("preset");
  delay.set_val("description", "Digital delay (before coincidence logic)");
  delay.set_val("units", "ns");
  delay.set_val("min", 0);
  det.branches.add(delay);

  Setting dets({"per_detector", SettingType::stem});
  dets.branches.add(det);
  attributes.branches.add(dets);

  SettingMeta coinc_window("coinc_window", SettingType::floating);
  coinc_window.set_flag("preset");
  coinc_window.set_val("description", "Coincidence window");
  coinc_window.set_val("units", "ns");
  coinc_window.set_val("min", 0);
  Setting coinc(coinc_window);
  coinc.set_number(50);
  attributes.branches.add(coinc);

  SettingMeta pattern_coinc("pattern_coinc", SettingType::pattern);
  pattern_coinc.set_flag("preset");
  pattern_coinc.set_val("description", "Coincidence pattern");
  pattern_coinc.set_val("chans", 1);
  attributes.branches.add(pattern_coinc);

  SettingMeta pattern_anti("pattern_anti", SettingType::pattern);
  pattern_anti.set_flag("preset");
  pattern_anti.set_val("description", "Anti-coincidence pattern");
  pattern_anti.set_val("chans", 1);
  attributes.branches.add(pattern_anti);

  SettingMeta pattern_add("pattern_add", SettingType::pattern);
  pattern_add.set_flag("preset");
  pattern_add.set_val("description", "Add pattern");
  pattern_add.set_val("chans", 1);
  attributes.branches.add(pattern_add);

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

bool Spectrum::_initialize()
{
  Consumer::_initialize();

  pattern_coinc_ = metadata_.get_attribute("pattern_coinc").pattern();
  pattern_anti_ = metadata_.get_attribute("pattern_anti").pattern();
  pattern_add_ = metadata_.get_attribute("pattern_add").pattern();
  coinc_window_ = metadata_.get_attribute("coinc_window").get_number();
  bits_ = metadata_.get_attribute("resolution").selection();
  if (coinc_window_ < 0)
    coinc_window_ = 0;

  max_delay_ = 0;
  Setting perdet = metadata_.get_attribute("per_detector");
  cutoff_logic_.resize(perdet.branches.size());
  delay_ns_.resize(perdet.branches.size());
  for (auto &d : perdet.branches)
  {
    int idx = -1;
    if (d.indices().size())
      idx = *d.indices().begin();
    if (idx >= static_cast<int16_t>(cutoff_logic_.size()))
    {
      cutoff_logic_.resize(idx + 1);
      delay_ns_.resize(idx + 1);
    }
    if (idx >= 0)
    {
      cutoff_logic_[idx] = d.find({"cutoff_logic"}, Match::id).get_number();
      delay_ns_[idx]     = d.find({"delay_ns"}, Match::id).get_number();
      if (delay_ns_[idx] > max_delay_)
        max_delay_ = delay_ns_[idx];
    }
  }
  max_delay_ += coinc_window_;
  //   DBG << "<" << metadata_.name << "> coinc " << coinc_window_ << " max delay " << max_delay_;

  return false; //still too abstract
}


void Spectrum::_push_hit(const Event& newhit)
{
  if ((newhit.channel() < 0)
      || (newhit.channel() >= static_cast<int16_t>(energy_idx_.size())))
    return;

  if ((newhit.channel() < static_cast<int16_t>(cutoff_logic_.size()))
      && (newhit.value(energy_idx_.at(newhit.channel())).val(bits_) < cutoff_logic_[newhit.channel()]))
    return;

  if (newhit.channel() < 0)
    return;

  if (!(pattern_coinc_.relevant(newhit.channel()) ||
        pattern_anti_.relevant(newhit.channel()) ||
        pattern_add_.relevant(newhit.channel())))
    return;

  //  DBG << "Processing " << newhit.to_string();

  Event hit = newhit;
  if (hit.channel() < static_cast<int16_t>(delay_ns_.size()))
    hit.delay_ns(delay_ns_[hit.channel()]);

  bool appended = false;
  bool pileup = false;
  if (backlog.empty() || backlog.back().past_due(hit))
    backlog.push_back(Coincidence(hit, coinc_window_, max_delay_));
  else
  {
    for (auto &q : backlog)
    {
      if (q.in_window(hit))
      {
        if (q.add_hit(hit))
        {
          if (appended)
            DBG << "<" << metadata_.get_attribute("name").get_text() << "> "
                << "hit " << hit.debug() << " coincident with more than one other hit (counted >=2 times)";
          appended = true;
        }
        else
        {
          DBG << "<" << metadata_.get_attribute("name").get_text() << "> "
              << "pileup hit " << hit.debug() << " with " << q.debug();
          pileup = true;
        }
      }
      else if (q.past_due(hit))
        break;
      else if (q.antecedent(hit))
        DBG << "<" << metadata_.get_attribute("name").get_text() << "> "
            << "antecedent hit " << hit.debug() << ". Something wrong with presorter or daq_device?";
    }

    if (!appended && !pileup)
    {
      backlog.push_back(Coincidence(hit, coinc_window_, max_delay_));
      //      DBG << "append fresh";
    }
  }

  Coincidence evt;
  while (!backlog.empty() && (evt = backlog.front()).past_due(hit))
  {
    backlog.pop_front();
    if (validate_coincidence(evt))
    {
      recent_count_++;
      total_events_++;
      this->add_coincicence(evt);
    }
  }

}


bool Spectrum::validate_coincidence(const Coincidence &newEvent) const
{
  if (!validate(newEvent, pattern_coinc_))
    return false;
  if (!antivalidate(newEvent, pattern_anti_))
    return false;
  return true;
}

void Spectrum::_push_stats(const Status& newBlock)
{
  //private; no lock required

  if (!(
        pattern_coinc_.relevant(newBlock.channel()) ||
        pattern_anti_.relevant(newBlock.channel()) ||
        pattern_add_.relevant(newBlock.channel())
        ))
    return;

  //DBG << "Spectrum " << metadata_.name << " received update for chan " << newBlock.channel;
  bool chan_new = (stats_list_.count(newBlock.channel()) == 0);
  bool new_start = (newBlock.type() == StatusType::start);

  if (newBlock.channel() >= static_cast<int16_t>(energy_idx_.size()))
    energy_idx_.resize(newBlock.channel() + 1, -1);
  if (newBlock.hit_model().name_to_val.count("energy"))
    energy_idx_[newBlock.channel()] = newBlock.hit_model().name_to_val.at("energy");

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

  metadata_.set_attribute(Setting::precise("total_hits", total_hits_), false);
  metadata_.set_attribute(Setting::precise("total_events", total_events_), false);
}


void Spectrum::_flush()
{
  metadata_.set_attribute(Setting::precise("total_hits", total_hits_), false);
  metadata_.set_attribute(Setting::precise("total_events", total_events_), false);
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
    uint32_t res = pow(2,bits_);
    axes_[i].resize(res, 0.0);
//    for (uint32_t j=0; j<res; j++)
//      axes_[i][j] = this_calib.transform(j, bits_);
  }
}
