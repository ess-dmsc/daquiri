#include "spectrum_time.h"
#include "sparse_matrix2d.h"

#include "custom_logger.h"

#define kDimensions 2

TimeSpectrum::TimeSpectrum()
{
  data_ = std::make_shared<SparseMatrix2D>(); //use dense 2d

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Spectra in time series");

  SettingMeta res("time_resolution", SettingType::floating);
  res.set_flag("preset");
  res.set_val("min", 1);
  res.set_val("units", "units (see below)");
  base_options.branches.add(res);

  SettingMeta units("units", SettingType::menu);
  units.set_flag("preset");
  units.set_enum(0, "ns");
  units.set_enum(3, "\u03BCs");
  units.set_enum(6, "ms");
  units.set_enum(9, "s");
  units.set_val("description", "Domain scale");
  base_options.branches.add(units);

  SettingMeta ds("downsample", SettingType::integer);
  ds.set_val("units", "bits");
  ds.set_flag("preset");
  ds.set_val("min", 0);
  ds.set_val("max", 31);
  base_options.branches.add(ds);

  SettingMeta val_name("value_name", SettingType::text);
  val_name.set_flag("preset");
  val_name.set_val("description", "Name of event value to bin");
  base_options.branches.add(val_name);

  SettingMeta add_channels("add_channels", SettingType::pattern, "Channels to bin");
  add_channels.set_flag("preset");
  add_channels.set_val("chans", 1);
  base_options.branches.add(add_channels);

  metadata_.overwrite_all_attributes(base_options);
}

bool TimeSpectrum::_initialize()
{
  Spectrum::_initialize();

  time_resolution_ = 1.0 / metadata_.get_attribute("time_resolution").get_number();
  downsample_ = metadata_.get_attribute("downsample").get_number();
  val_name_ = metadata_.get_attribute("value_name").get_text();
  channels_ = metadata_.get_attribute("add_channels").pattern();

  auto unit = metadata_.get_attribute("units").selection();
  units_name_ = metadata_.get_attribute("units").metadata().enum_name(unit);
  units_multiplier_ = std::pow(10, unit);

  time_resolution_ /= units_multiplier_;

  DBG << "Time resolution " << metadata_.get_attribute("time_resolution").get_number()
      << " " << units_name_;
  DBG << "Units multiplier = ns / 10^" << unit
      << "  =  ns / " << units_multiplier_
      << "  =  ns * " << time_resolution_;

  DBG << "One bin = " << 1.0 / time_resolution_ / units_multiplier_;

  int adds = 1;//0;
  //  std::vector<bool> gts = add_channels_.gates();
  //  for (size_t i=0; i < gts.size(); ++i)
  //    if (gts[i])
  //      adds++;

  if (adds != 1)
  {
    WARN << "<TimeSpectrum> Cannot initialize. Add pattern must have 1 selected channel.";
    return false;
  }

  this->_recalc_axes();
  return true;
}

void TimeSpectrum::_init_from_file()
{
  channels_.resize(1);
  channels_.set_gates(std::vector<bool>({true}));
  metadata_.set_attribute(Setting("add_channels", channels_));
  metadata_.set_attribute(Setting::integer("value_downsample", downsample_));

  Spectrum::_init_from_file();
}

void TimeSpectrum::_set_detectors(const std::vector<Detector>& dets)
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

void TimeSpectrum::_recalc_axes()
{
  Detector det;
  if (metadata_.detectors.size() == 1)
    det = metadata_.detectors[0];
  CalibID from(det.id(), val_name_, "");
  CalibID to("", val_name_, "");
  auto calib = det.get_calibration(from, to);
  data_->set_axis(1, DataAxis(calib));

  data_->recalc_axes();

  CalibID id("", "time", units_name_);
  DataAxis ax;
  ax.calibration = Calibration(id, id);
  ax.domain = domain_;
  data_->set_axis(0, ax);

  DBG << "Axes recalced "
      << data_->axis(0).debug() << " x " << data_->axis(1).debug();
}

bool TimeSpectrum::channel_relevant(int16_t channel) const
{
  return ((channel >= 0) && channels_.relevant(channel));
}

void TimeSpectrum::_push_event(const Event& e)
{
  const auto& c = e.channel();

  if (!this->channel_relevant(c)
      || !value_relevant(c, value_idx_)
      || !time_resolution_)
    return;

  double nsecs = e.timestamp().nanosecs();

  coords_[0] = static_cast<size_t>(std::round(nsecs * time_resolution_));

  if (downsample_)
    coords_[1] = (e.value(value_idx_[c]) >> downsample_);
  else
    coords_[1] = e.value(value_idx_[c]);

  if (coords_[0] >= domain_.size())
  {
    size_t oldbound = domain_.size();
    domain_.resize(coords_[0]+1);

    for (size_t i=oldbound; i <= coords_[0]; ++i)
      domain_[i] = i / time_resolution_ / units_multiplier_;

    DBG << "Resized domain to " << domain_.size()
        << " (" << domain_[domain_.size()-1] << ")"
        << " because " << e.timestamp().debug() << " * " << time_resolution_
        << " --> " << coords_[0];
  }

  data_->add_one(coords_);

  total_count_++;
  recent_count_++;
}


void TimeSpectrum::_push_stats(const Status& newBlock)
{
  if (!this->channel_relevant(newBlock.channel()))
    return;

  Spectrum::_push_stats(newBlock);

  if (newBlock.channel() >= static_cast<int16_t>(value_idx_.size()))
    value_idx_.resize(newBlock.channel() + 1, -1);
  if (newBlock.event_model().name_to_val.count(val_name_))
    value_idx_[newBlock.channel()] = newBlock.event_model().name_to_val.at(val_name_);

  return;

  if (!this->channel_relevant(newBlock.channel()))
    return;

  if (newBlock.channel() >= static_cast<int16_t>(value_idx_.size()))
    value_idx_.resize(newBlock.channel() + 1, -1);
  if (newBlock.event_model().name_to_val.count(val_name_))
    value_idx_[newBlock.channel()] = newBlock.event_model().name_to_val.at(val_name_);

  PreciseFloat real = 0;
  PreciseFloat live = 0;
  PreciseFloat percent_dead = 0;
  PreciseFloat tot_time = 0;

//  spectra_.push_back(std::vector<PreciseFloat>(pow(2, bits_)));

  if (!updates_.empty())
  {
    if ((newBlock.type() == StatusType::stop) &&
        (updates_.back().type() == StatusType::running))
    {
      updates_.pop_back();
      seconds_.pop_back();
    }

    boost::posix_time::time_duration rt ,lt;
    lt = rt = newBlock.time() - updates_.back().time();

    Status diff;// = newBlock - updates_.back();
    PreciseFloat scale_factor = 1;
    if (diff.stats().count("native_time") && (diff.stats().at("native_time") > 0))
      scale_factor = rt.total_microseconds() / diff.stats()["native_time"];

    if (diff.stats().count("live_time"))
    {
      PreciseFloat scaled_live = diff.stats().at("live_time") * scale_factor;
      lt = boost::posix_time::microseconds(static_cast<long>(to_double(scaled_live)));
    }

    real     = rt.total_milliseconds()  * 0.001;
    tot_time = (newBlock.time() - updates_.front().time()).total_milliseconds() * 0.001;

    live = lt.total_milliseconds() * 0.001;

    percent_dead = (real - live) / real * 100;
  }


  if (seconds_.empty() || (tot_time != 0))
  {
    seconds_.push_back(to_double(tot_time));
    updates_.push_back(newBlock);

    //      spectrum_.push_back(count);

//    axes_[0].clear();
//    for (auto &q : seconds_)
//      axes_[0].push_back(to_double(q));
  }

  Spectrum::_push_stats(newBlock);
}


