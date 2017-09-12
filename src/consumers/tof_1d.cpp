#include "tof_1d.h"
#include "dense1d.h"

#include "custom_logger.h"

#define kDimensions 1

TOF1D::TOF1D()
  : Spectrum()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Event mode 1D spectrum");

  SettingMeta app("appearance", SettingType::color);
  app.set_val("description", "Plot appearance");
  base_options.branches.add(Setting(app));

  SettingMeta res("resolution", SettingType::floating);
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

  SettingMeta add_channels("add_channels", SettingType::pattern, "Channels to bin");
  add_channels.set_flag("preset");
  add_channels.set_val("chans", 1);
  base_options.branches.add(add_channels);

  metadata_.overwrite_all_attributes(base_options);
}

bool TOF1D::_initialize()
{
  Spectrum::_initialize();
  resolution_ = 1.0 / metadata_.get_attribute("resolution").get_number();
  channels_ = metadata_.get_attribute("add_channels").pattern();

  auto unit = metadata_.get_attribute("units").selection();
  units_name_ = metadata_.get_attribute("units").metadata().enum_name(unit);
  units_multiplier_ = std::pow(10, unit);

  resolution_ /= units_multiplier_;

  this->_recalc_axes();
  return true;
}

void TOF1D::_init_from_file()
{
//  metadata_.set_attribute(Setting::integer("resolution", bits_));

  channels_.resize(1);
  channels_.set_gates(std::vector<bool>({true}));
  metadata_.set_attribute(Setting("add_channels", channels_));

  Spectrum::_init_from_file();
}

void TOF1D::_set_detectors(const std::vector<Detector>& dets)
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

void TOF1D::_recalc_axes()
{
  CalibID id("", "time", units_name_, 0);
  DataAxis ax;
  ax.calibration = Calibration(id, id);
  ax.domain = domain_;
  data_->set_axis(0, ax);
}

bool TOF1D::channel_relevant(int16_t channel) const
{
  return ((channel >= 0) && channels_.relevant(channel));
}

void TOF1D::_push_spill(const Spill& spill)
{
  for (const auto& q : spill.stats)
  {
    if (this->channel_relevant(q.second.channel()) &&
        q.second.stats().count("pulse_time"))
      pulse_times_[q.second.channel()] = q.second.stats().at("pulse_time");
  }

  Spectrum::_push_spill(spill);
}


void TOF1D::_push_event(const Event& e)
{
  const auto& c = e.channel();

  if (!this->channel_relevant(c)
      || !pulse_times_.count(c)
      || !resolution_)
    return;

  double nsecs =
      TimeStamp(e.timestamp().native() - pulse_times_[c], e.timestamp().base()).nanosecs();

  if (nsecs < 0)
    return;

  size_t val = static_cast<size_t>(nsecs * resolution_);

  if (val >= domain_.size())
  {
    size_t oldbound = domain_.size();
    domain_.resize(val+1);

    for (size_t i=oldbound; i <= val; ++i)
      domain_[i] = i / resolution_ / units_multiplier_;
  }

  data_->add({{val}, 1});
  total_count_++;
  recent_count_++;
}
