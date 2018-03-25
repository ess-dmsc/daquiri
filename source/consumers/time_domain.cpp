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

  SettingMeta trim("trim", SettingType::boolean, "Trim last bin");
  base_options.branches.add(trim);

  base_options.branches.add(filters_.settings());

  metadata_.overwrite_all_attributes(base_options);
  //  DBG << "<TimeDomain:" << metadata_.get_attribute("name").value_text << ">  made with dims=" << metadata_.dimensions();
}

void TimeDomain::_apply_attributes()
{
  Spectrum::_apply_attributes();

  time_resolution_ = 1.0 / metadata_.get_attribute("time_resolution").get_number();
  auto unit = metadata_.get_attribute("time_units").selection();
  units_name_ = metadata_.get_attribute("time_units").metadata().enum_name(unit);
  units_multiplier_ = std::pow(10, unit);
  time_resolution_ /= units_multiplier_;

  trim_ = metadata_.get_attribute("trim").get_bool();

  filters_.settings(metadata_.get_attribute("filters"));
  metadata_.replace_attribute(filters_.settings());
}

void TimeDomain::_init_from_file()
{
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
//  CalibID id("time", "", units_name_);
//  data_->set_axis(0, DataAxis(Calibration(id, id), domain_));

  std::vector<double> domain(domain_.begin(), domain_.end());
//  for (auto& d : domain)
//    d /= units_multiplier_;

  size_t max = range_.size();
  if (trim_ && (max > 1))
  {
    max--;
    domain.resize(max);
  }

  data_->clear();
  CalibID id("time", "", units_name_);
  data_->set_axis(0, DataAxis(Calibration(id, id), domain));
  for (size_t i = 0; i < max; ++i)
  {
    entry_.first[0] = i;
    entry_.second = range_[i];
    data_->add(entry_);
  }
}

bool TimeDomain::_accept_spill(const Spill& spill)
{
  return (Spectrum::_accept_spill(spill));
}

bool TimeDomain::_accept_events(const Spill &spill)
{
  return (0 != time_resolution_);
}

void TimeDomain::_push_stats_pre(const Spill& spill)
{
  if (this->_accept_spill(spill))
  {
    timebase_ = spill.event_model.timebase;
    filters_.configure(spill);
    Spectrum::_push_stats_pre(spill);
  }
}

void TimeDomain::_push_event(const Event& event)
{
  if (!filters_.accept(event))
    return;

  double nsecs = timebase_.to_nanosec(event.timestamp());

  size_t bin = static_cast<size_t>(std::round(nsecs * time_resolution_));

  if (bin >= domain_.size())
  {
    size_t oldbound = domain_.size();
    domain_.resize(bin + 1);
    range_.resize(bin + 1, 0.0);

    for (size_t i = oldbound; i <= bin; ++i)
      domain_[i] = i / time_resolution_ / units_multiplier_;
  }

  range_[bin]++;
  total_count_++;
  recent_count_++;
}


