#include <consumers/tof_val_2d.h>
#include <consumers/dataspaces/sparse_map2d.h>

#include <core/util/custom_logger.h>

namespace DAQuiri {

TOFVal2D::TOFVal2D()
    : Spectrum()
{
  data_ = std::make_shared<SparseMap2D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Time of flight vs. value 2D spectrum");

  SettingMeta app("appearance", SettingType::text, "Plot appearance");
  app.set_flag("gradient-name");
  base_options.branches.add(app);

  SettingMeta flip("flip-y", SettingType::boolean, "Flip Y axis");
  base_options.branches.add(flip);

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

  base_options.branches.add(value_latch_.settings(-1, "Value to bin"));

  metadata_.overwrite_all_attributes(base_options);
}

void TOFVal2D::_apply_attributes()
{
  Spectrum::_apply_attributes();

  time_resolution_ = 0;
  if (metadata_.get_attribute("time_resolution").get_number() > 0)
    time_resolution_ = 1.0 / metadata_.get_attribute("time_resolution").get_number();
  auto unit = metadata_.get_attribute("time_units").selection();
  units_name_ = metadata_.get_attribute("time_units").metadata().enum_name(unit);
  units_multiplier_ = std::pow(10, unit);
  time_resolution_ /= units_multiplier_;

  value_latch_.settings(metadata_.get_attribute("value_latch"));
  metadata_.replace_attribute(value_latch_.settings(-1, "Value to bin"));

  this->_recalc_axes();
}

void TOFVal2D::_init_from_file()
{
  domain_ = data_->axis(0).domain;
  Spectrum::_init_from_file();
}

void TOFVal2D::_recalc_axes()
{
  Detector det;
  if (metadata_.detectors.size() == 1)
    det = metadata_.detectors[0];

  auto calib = det.get_calibration({value_latch_.value_id, det.id()}, {value_latch_.value_id});
  data_->set_axis(1, DataAxis(calib, value_latch_.downsample));

  data_->recalc_axes();

  CalibID id("time", "", units_name_);
  data_->set_axis(0, DataAxis(Calibration(id, id), domain_));
}

bool TOFVal2D::_accept_spill(const Spill& spill)
{
  return (Spectrum::_accept_spill(spill)
      && value_latch_.has_declared_value(spill));
}

void TOFVal2D::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;
  timebase_ = spill.event_model.timebase;
  pulse_time_ = timebase_.to_nanosec(
      spill.state.find(Setting("pulse_time")).get_number());
  value_latch_.configure(spill);
  Spectrum::_push_stats_pre(spill);
}

bool TOFVal2D::_accept_events(const Spill& /*spill*/)
{
  return value_latch_.valid() &&
      (0 != time_resolution_) &&
      (pulse_time_ >= 0);
}

void TOFVal2D::_push_event(const Event& event)
{
  if (!filters_.accept(event))
    return;

  double nsecs = timebase_.to_nanosec(event.timestamp()) - pulse_time_;

  if (nsecs < 0)
    return;

  coords_[0] = static_cast<size_t>(nsecs * time_resolution_);

  value_latch_.extract(coords_[1], event);

  if (coords_[0] >= domain_.size())
  {
    size_t oldbound = domain_.size();
    domain_.resize(coords_[0] + 1);

    for (size_t i = oldbound; i <= coords_[0]; ++i)
      domain_[i] = i / time_resolution_ / units_multiplier_;
  }

  data_->add_one(coords_);
}

}
