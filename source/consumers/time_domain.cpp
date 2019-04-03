#include <consumers/time_domain.h>
#include <consumers/dataspaces/dense1d.h>

#include <core/util/logger.h>

namespace DAQuiri {

TimeDomain::TimeDomain()
    : Spectrum()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Time-domain log of activity");

  SettingMeta app("appearance", SettingType::text, "Appearance");
  app.set_flag("color");
  base_options.branches.add(Setting(app));

  SettingMeta win("window", SettingType::floating, "Window size");
  win.set_flag("preset");
  win.set_val("min", 1);
  win.set_val("units", "units (see below)");
  win.set_val("description", "Infinite if =0");
  base_options.branches.add(win);

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

  SettingMeta trim("trim", SettingType::boolean, "Trim last bin");
  base_options.branches.add(trim);

  metadata_.overwrite_all_attributes(base_options);
}

void TimeDomain::_apply_attributes()
{
  Spectrum::_apply_attributes();

  time_resolution_ = 0;
  if (metadata_.get_attribute("time_resolution").get_number() > 0)
    time_resolution_ = 1.0 / metadata_.get_attribute("time_resolution").get_number();
  auto unit = metadata_.get_attribute("time_units").selection();
  units_name_ = metadata_.get_attribute("time_units").metadata().enum_name(unit);
  units_multiplier_ = std::pow(10, unit);
  time_resolution_ /= units_multiplier_;

  window_ = metadata_.get_attribute("window").get_number() * units_multiplier_;
  trim_ = metadata_.get_attribute("trim").get_bool();
}

void TimeDomain::_init_from_file()
{
  domain_ = data_->axis(0).domain;
  for (auto& d : domain_)
    d *= units_multiplier_;

  range_.resize(domain_.size(), 0.0);
  auto data = data_->all_data();
  for (auto e : *data)
    range_[e.first[0]] = static_cast<double>(e.second);
  Spectrum::_init_from_file();
}

void TimeDomain::_recalc_axes()
{
  std::vector<double> domain(domain_.begin(), domain_.end());
  for (auto& d : domain)
    d /= units_multiplier_;

  data_->clear();
  CalibID id("time", "", units_name_);
  data_->set_axis(0, DataAxis(Calibration(id, id), domain));
  for (size_t i = 0; i < range_.size(); ++i)
  {
    entry_.first[0] = i;
    entry_.second = range_[i];
    data_->add(entry_);
  }
}

bool TimeDomain::_accept_events(const Spill& /*spill*/)
{
  return (0 != time_resolution_);
}

void TimeDomain::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  timebase_ = spill.event_model.timebase;
  Spectrum::_push_stats_pre(spill);
}

void TimeDomain::_push_event(const Event& event)
{
  if (!filters_.accept(event))
    return;

  double nsecs = timebase_.to_nanosec(event.timestamp());

  if (nsecs < earliest_)
    return;

  if (window_ > 0.0)
  {
    while (domain_.size() && (domain_[0] < (nsecs - window_)))
    {
      domain_.erase(domain_.begin());
      range_.erase(range_.begin());
    }

    if (domain_.size())
      earliest_ = domain_[0];
  }

  size_t bin = static_cast<size_t>(std::round((nsecs - earliest_) * time_resolution_));

  if (bin >= domain_.size())
  {
    size_t oldbound = domain_.size();
    domain_.resize(bin + 1);
    range_.resize(bin + 1, 0.0);

    for (size_t i = oldbound; i <= bin; ++i)
      domain_[i] = i / time_resolution_ + earliest_;
  }

  range_[bin]++;
}

}
