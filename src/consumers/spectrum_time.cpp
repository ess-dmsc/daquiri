#include "spectrum_time.h"
#include "sparse_matrix2d.h"

#include "custom_logger.h"

#define kDimensions 2

TimeSpectrum::TimeSpectrum()
{
  data_ = std::make_shared<SparseMatrix2D>(); //use dense 2d

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Spectra in time series");

  SettingMeta res("time_resolution", SettingType::floating, "Time resolution");
  res.set_flag("preset");
  res.set_val("min", 1);
  res.set_val("units", "units (see below)");
  base_options.branches.add(res);

  SettingMeta units("time_units", SettingType::menu);
  units.set_flag("preset");
  units.set_enum(0, "ns");
  units.set_enum(3, "\u03BCs");
  units.set_enum(6, "ms");
  units.set_enum(9, "s");
  units.set_val("description", "Domain scale");
  base_options.branches.add(units);

  SettingMeta ds("downsample", SettingType::integer, "Downsample by");
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

  auto unit = metadata_.get_attribute("time_units").selection();
  units_name_ = metadata_.get_attribute("time_units").metadata().enum_name(unit);
  units_multiplier_ = std::pow(10, unit);

  time_resolution_ /= units_multiplier_;

//  DBG << "Time resolution " << metadata_.get_attribute("time_resolution").get_number()
//      << " " << units_name_;
//  DBG << "Units multiplier = ns / 10^" << unit
//      << "  =  ns / " << units_multiplier_
//      << "  =  ns * " << time_resolution_;

//  DBG << "One bin = " << 1.0 / time_resolution_ / units_multiplier_;

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

  auto calib = det.get_calibration({val_name_, det.id()}, {val_name_});
  data_->set_axis(1, DataAxis(calib, downsample_));

  data_->recalc_axes();

  CalibID id("", "time", units_name_);
  data_->set_axis(0, DataAxis(Calibration(id, id), domain_));
}

bool TimeSpectrum::channel_relevant(int16_t channel) const
{
  return ((channel >= 0) && channels_.relevant(channel));
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
  }

  data_->add_one(coords_);

  total_count_++;
  recent_count_++;
}


