#include "time_domain.h"
#include "custom_logger.h"
#include "dense1d.h"

#define kDimensions 1

TimeDomain::TimeDomain()
  : Spectrum()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Time-domain log of activity");

  SettingMeta app("appearance", SettingType::color);
  app.set_val("description", "Plot appearance");
  base_options.branches.add(Setting(app));

  SettingMeta res("co-domain", SettingType::menu);
  res.set_flag("preset");
  res.set_enum(0, "event rate");
  res.set_enum(1, "% dead-time");
  res.set_val("description", "Choice of dependent variable");
  base_options.branches.add(res);

  SettingMeta pattern_add("channels", SettingType::pattern);
  pattern_add.set_flag("preset");
  pattern_add.set_val("description", "Channels to bin");
  pattern_add.set_val("chans", 1);
  base_options.branches.add(pattern_add);

  metadata_.overwrite_all_attributes(base_options);
  //  DBG << "<TimeDomain:" << metadata_.get_attribute("name").value_text << ">  made with dims=" << metadata_.dimensions();
}

bool TimeDomain::_initialize()
{
  Spectrum::_initialize();

  channels_ = metadata_.get_attribute("channels").pattern();
  codomain = metadata_.get_attribute("co-domain").selection();

  int adds = 1; //0;
  //  std::vector<bool> gts = pattern_add_.gates();
  //  for (size_t i=0; i < gts.size(); ++i)
  //    if (gts[i])
  //      adds++;

  if (adds != 1)
  {
    WARN << "<TimeDomain> Cannot initialize. Add pattern must have 1 selected channel.";
    return false;
  }

  return true;
}

void TimeDomain::_init_from_file()
{
  channels_.resize(1);
  channels_.set_gates(std::vector<bool>({true}));
  metadata_.set_attribute(Setting("channels", channels_));

  Spectrum::_init_from_file();
}


void TimeDomain::_set_detectors(const std::vector<Detector>& dets)
{
  metadata_.detectors.resize(kDimensions, Detector());

  if (dets.size() == kDimensions)
    metadata_.detectors = dets;

  if (dets.size() >= kDimensions)
  {
    for (size_t i=0; i < dets.size(); ++i)
    {
      if (metadata_.chan_relevant(i))
      {
        metadata_.detectors[0] = dets[i];
        break;
      }
    }
  }

  this->_recalc_axes();
}

void TimeDomain::_recalc_axes()
{
  CalibID id("", "time", "s", 0);
  DataAxis ax;
  ax.calibration = Calibration(id, id);
  ax.domain = seconds_;
  data_->set_axis(0, ax);
}

bool TimeDomain::channel_relevant(int16_t channel) const
{
  return ((channel >= 0) && channels_.relevant(channel));
}

void TimeDomain::_push_event(const Event& e)
{
  const auto& c = e.channel();

  if (!this->channel_relevant(c))
    return;

  total_count_++;
  recent_count_++;
}

void TimeDomain::_push_stats(const Status& status)
{
  if (!this->channel_relevant(status.channel()))
    return;

  PreciseFloat real = 0;
  PreciseFloat live = 0;
  PreciseFloat percent_dead = 0;
  PreciseFloat tot_time = 0;

  if (!updates_.empty())
  {
    Status start = updates_.back();
    if ((status.type() == StatusType::stop) &&
        (start.type() == StatusType::running))
    {
      updates_.pop_back();
      seconds_.pop_back();
      spectrum_.pop_back();
      counts_.push_back(recent_count_ + counts_.back());
    }
    else
      counts_.push_back(recent_count_);

    PreciseFloat diff_native{0}, diff_live{0};
    if (status.stats().count("native_time") && start.stats().count("native_time"))
      diff_native = status.stats().at("native_time") - start.stats().at("native_time");
    if (status.stats().count("live_time") && start.stats().count("live_time"))
      diff_live = status.stats().at("live_time") - start.stats().at("live_time");

    boost::posix_time::time_duration rt ,lt;
    lt = rt = status.time() - start.time();

    PreciseFloat scale_factor = 1;
    if (diff_native > 0)
      scale_factor = rt.total_microseconds() / diff_native;
    if (diff_live)
    {
      PreciseFloat scaled_live = diff_live * scale_factor;
      lt = boost::posix_time::microseconds(static_cast<double>(scaled_live));
    }

    real     = rt.total_milliseconds()  * 0.001;
    tot_time = (status.time() - updates_.front().time()).total_milliseconds() * 0.001;

    live = lt.total_milliseconds() * 0.001;

    percent_dead = (real - live) / real * 100;

  } else
    counts_.push_back(recent_count_);

  DBG << "Live time = " << live
      << " Real time = " << live
      << " dead% " << percent_dead
      << " for " << codomain;

  if (seconds_.empty() || (tot_time != 0))
  {
    PreciseFloat count = 0;

    if (codomain == 0)
    {
      count = counts_.back();
      if (live > 0)
        count /= live;
    }
    else if (codomain == 1)
    {
      count = percent_dead;
    }

    seconds_.push_back(to_double(tot_time));
    updates_.push_back(status);

    spectrum_.push_back(count);

    //      DBG << "<TimeDomain> \"" << metadata_.name << "\" chan " << int(status.channel) << " nrgs.size="
    //             << energies_[0].size() << " nrgs.last=" << energies_[0][energies_[0].size()-1]
    //             << " spectrum.size=" << spectrum_.size() << " spectrum.last=" << spectrum_[spectrum_.size()-1].convert_to<double>();
  }

  data_->clear();
  for (size_t i=0; i < spectrum_.size(); ++i)
    data_->add({{i}, spectrum_[i]});

  Spectrum::_push_stats(status);
}


