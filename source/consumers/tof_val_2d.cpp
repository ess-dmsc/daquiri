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

  SettingMeta app("appearance", SettingType::text, "Plot appearance");
  app.set_flag("gradient-name");
  base_options.branches.add(Setting(app));

  SettingMeta res("time_resolution", SettingType::floating, "Time resolution");
  res.set_flag("preset");
  res.set_val("min", 1);
  res.set_val("units", "units (see below)");
  base_options.branches.add(res);

  SettingMeta units("time_units", SettingType::menu, "Time units (domain)");
  units.set_flag("preset");
  units.set_enum(0, "ns");
  units.set_enum(3, "\u03BCs");
  units.set_enum(6, "ms");
  units.set_enum(9, "s");
  base_options.branches.add(units);

  SettingMeta val_name("value_name", SettingType::text);
  val_name.set_flag("preset");
  val_name.set_flag("event_value");
  val_name.set_val("description", "Name of event value to bin");
  base_options.branches.add(val_name);

  SettingMeta ds("downsample", SettingType::integer, "Downsample value by");
  ds.set_val("units", "bits");
  ds.set_flag("preset");
  ds.set_val("min", 0);
  ds.set_val("max", 31);
  base_options.branches.add(ds);

  metadata_.overwrite_all_attributes(base_options);
}

void TOFVal2D::_apply_attributes()
{
  Spectrum::_apply_attributes();
  time_resolution_ = 1.0 / metadata_.get_attribute("time_resolution").get_number();
  val_name_ = metadata_.get_attribute("value_name").get_text();
  downsample_ = metadata_.get_attribute("value_downsample").get_number();

  auto unit = metadata_.get_attribute("time_units").selection();
  units_name_ = metadata_.get_attribute("time_units").metadata().enum_name(unit);
  units_multiplier_ = std::pow(10, unit);

  time_resolution_ /= units_multiplier_;

  this->_recalc_axes();
}

void TOFVal2D::_init_from_file()
{
//  metadata_.set_attribute(Setting::integer("resolution", bits_));

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

bool TOFVal2D::_accept_spill(const Spill& spill)
{
  return (Spectrum::_accept_spill(spill)
          && spill.event_model.name_to_val.count(val_name_));
}

bool TOFVal2D::_accept_events(const Spill &spill)
{
  return (value_idx_ >= 0) &&
      (0 != time_resolution_) &&
      (pulse_time_ >= 0);
}

void TOFVal2D::_push_stats_pre(const Spill& spill)
{
  if (this->_accept_spill(spill))
  {
    timebase_ = spill.event_model.timebase;
    value_idx_ = spill.event_model.name_to_val.at(val_name_);
    pulse_time_ = timebase_.to_nanosec(
          spill.state.find(Setting("pulse_time")).get_number());

    Spectrum::_push_stats_pre(spill);
  }
}

void TOFVal2D::_push_event(const Event& e)
{
  double nsecs = timebase_.to_nanosec(e.timestamp()) - pulse_time_;

  if (nsecs < 0)
    return;

  coords_[0] = static_cast<size_t>(nsecs * time_resolution_);

  if (downsample_)
    coords_[1] = (e.value(value_idx_) >> downsample_);
  else
    coords_[1] = e.value(value_idx_);

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
