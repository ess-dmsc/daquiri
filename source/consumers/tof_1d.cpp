#include <consumers/tof_1d.h>
#include <consumers/dataspaces/dense1d.h>

#include <core/util/logger.h>

namespace DAQuiri {

TOF1D::TOF1D()
    : Spectrum()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Time of flight 1D spectrum");

  SettingMeta app("appearance", SettingType::text, "Appearance");
  app.set_flag("color");
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

  metadata_.overwrite_all_attributes(base_options);
}

void TOF1D::_apply_attributes()
{
  try
  {
    Spectrum::_apply_attributes();

    time_resolution_ = 0;
    if (metadata_.get_attribute("time_resolution").get_number() > 0)
      time_resolution_ = 1.0 / metadata_.get_attribute("time_resolution").get_number();
    auto unit = metadata_.get_attribute("time_units").selection();
    units_name_ = metadata_.get_attribute("time_units").metadata().enum_name(unit);
    units_multiplier_ = std::pow(10, unit);
    time_resolution_ /= units_multiplier_;

    this->_recalc_axes();
  }
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("<TOF1D> Failed _apply_attributes"));
  }
}

void TOF1D::_init_from_file()
{
  try
  {
    domain_ = data_->axis(0).domain;
    Spectrum::_init_from_file();
  }
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("<TOF1D> Failed _init_from_file"));
  }
}

void TOF1D::_recalc_axes()
{
  try
  {
    CalibID id("time", "", units_name_);
    data_->set_axis(0, DataAxis(Calibration(id, id), domain_));
  }
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("<TOF1D> Failed _recalc_axes"));
  }
}

bool TOF1D::_accept_spill(const Spill& spill)
{
  return (Spectrum::_accept_spill(spill));
}

void TOF1D::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;
  timebase_ = spill.event_model.timebase;
  pulse_time_ = timebase_.to_nanosec(
      spill.state.find(Setting("pulse_time")).get_number());
  Spectrum::_push_stats_pre(spill);
}

bool TOF1D::_accept_events(const Spill& /*spill*/)
{
  return ((pulse_time_ >= 0) && (0 != time_resolution_));
}

void TOF1D::_push_event(const Event& event)
{
  if (!filters_.accept(event))
    return;

  double nsecs = timebase_.to_nanosec(event.timestamp()) - pulse_time_;

  if (nsecs < 0)
    return;

  coords_[0] = static_cast<size_t>(nsecs * time_resolution_);

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
