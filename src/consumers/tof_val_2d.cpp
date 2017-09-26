#include "tof_val_2d.h"
#include "sparse_map2d.h"

#include "custom_logger.h"

#define kDimensions 2

TOFVal2D::TOFVal2D()
  : Spectrum()
{
  data_ = std::make_shared<SparseMap2D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Time of flight vs. value 2D spectrum");

  SettingMeta res("time_resolution", SettingType::floating, "Time resolution");
  res.set_flag("preset");
  res.set_val("min", 1);
  res.set_val("units", "units (see below)");
  base_options.branches.add(res);

  SettingMeta units("time_units", SettingType::menu, "Time units");
  units.set_flag("preset");
  units.set_enum(0, "ns");
  units.set_enum(3, "\u03BCs");
  units.set_enum(6, "ms");
  units.set_enum(9, "s");
  units.set_val("description", "Domain scale");
  base_options.branches.add(units);

  SettingMeta val_name("value_name", SettingType::text);
  val_name.set_flag("preset");
  val_name.set_val("description", "Name of event value to bin");
  base_options.branches.add(val_name);

  SettingMeta ds("downsample", SettingType::integer, "Downsample value by");
  ds.set_val("units", "bits");
  ds.set_flag("preset");
  ds.set_val("min", 0);
  ds.set_val("max", 31);
  base_options.branches.add(ds);

  SettingMeta add_channels("add_channels", SettingType::pattern, "Channels to bin");
  add_channels.set_flag("preset");
  add_channels.set_val("chans", 1);
  base_options.branches.add(add_channels);

  metadata_.overwrite_all_attributes(base_options);
}

bool TOFVal2D::_initialize()
{
  Spectrum::_initialize();
  time_resolution_ = 1.0 / metadata_.get_attribute("time_resolution").get_number();
  channels_ = metadata_.get_attribute("add_channels").pattern();
  val_name_ = metadata_.get_attribute("value_name").get_text();
  downsample_ = metadata_.get_attribute("value_downsample").get_number();

  auto unit = metadata_.get_attribute("time_units").selection();
  units_name_ = metadata_.get_attribute("time_units").metadata().enum_name(unit);
  units_multiplier_ = std::pow(10, unit);

  time_resolution_ /= units_multiplier_;

  this->_recalc_axes();
  return true;
}

void TOFVal2D::_init_from_file()
{
//  metadata_.set_attribute(Setting::integer("resolution", bits_));

  channels_.resize(1);
  channels_.set_gates(std::vector<bool>({true}));
  metadata_.set_attribute(Setting("add_channels", channels_));
  metadata_.set_attribute(Setting::integer("value_downsample", downsample_));

  Spectrum::_init_from_file();
}

void TOFVal2D::_set_detectors(const std::vector<Detector>& dets)
{
  metadata_.detectors.resize(1, Detector());

  if (dets.size() == 1)
    metadata_.detectors = dets;

  this->_recalc_axes();
}

void TOFVal2D::_recalc_axes()
{
  Detector det;
  if (metadata_.detectors.size() == 1)
    det = metadata_.detectors[0];

  auto calib = det.get_calibration({val_name_, det.id()}, {val_name_});
  data_->set_axis(1, DataAxis(calib, downsample_));

  data_->recalc_axes();

  CalibID id("time", "", units_name_);
  data_->set_axis(0, DataAxis(Calibration(id, id), domain_));
}

bool TOFVal2D::channel_relevant(int16_t channel) const
{
  return ((channel >= 0) && channels_.relevant(channel));
}

void TOFVal2D::_push_spill(const Spill& spill)
{
  for (const auto& q : spill.stats)
  {
    if ((q.first < 0) ||
        !this->channel_relevant(q.second.channel()))
      continue;

    timebase_.resize(q.first + 1);
    pulse_times_.resize(q.first + 1, -1);

    timebase_[q.first] = q.second.event_model().timebase;
    if (q.second.stats().count("pulse_time"))
      pulse_times_[q.first] = timebase_[q.first].to_nanosec(q.second.stats()["pulse_time"]);
  }

  Spectrum::_push_spill(spill);
}

void TOFVal2D::_push_stats(const Status& newBlock)
{
  if (!this->channel_relevant(newBlock.channel()))
    return;

  Spectrum::_push_stats(newBlock);

  if (newBlock.channel() >= static_cast<int16_t>(value_idx_.size()))
    value_idx_.resize(newBlock.channel() + 1, -1);
  if (newBlock.event_model().name_to_val.count(val_name_))
    value_idx_[newBlock.channel()] = newBlock.event_model().name_to_val.at(val_name_);
}

void TOFVal2D::_push_event(const Event& e)
{
  const auto& c = e.channel();

  if (!this->channel_relevant(c)
      || !value_relevant(c, value_idx_)
      || (c < 0)
      || (c >= pulse_times_.size())
      || (pulse_times_[c] < 0)
      || !time_resolution_)
    return;

  double nsecs = timebase_[c].to_nanosec(e.timestamp()) - pulse_times_[c];

  if (nsecs < 0)
    return;

  coords_[0] = static_cast<size_t>(nsecs * time_resolution_);

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
