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
  res.set_val("units", "1/ns");
  base_options.branches.add(res);

  SettingMeta pattern_add("channels", SettingType::pattern);
  pattern_add.set_flag("preset");
  pattern_add.set_val("description", "Channels to bin");
  pattern_add.set_val("chans", 1);
  base_options.branches.add(pattern_add);

  metadata_.overwrite_all_attributes(base_options);
}

bool TOF1D::_initialize()
{
  Spectrum::_initialize();
  resolution_ = metadata_.get_attribute("resolution").get_number();
  channels_ = metadata_.get_attribute("channels").pattern();

  this->_recalc_axes();
  return true;
}

void TOF1D::_init_from_file()
{
//  metadata_.set_attribute(Setting::integer("resolution", bits_));

  channels_.resize(1);
  channels_.set_gates(std::vector<bool>({true}));
  metadata_.set_attribute(Setting("channels", channels_));

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
  CalibID id("", "time", "ns", 0);
  DataAxis ax;
  ax.calibration = Calibration(id, id);
  ax.domain = domain_;
  data_->set_axis(0, ax);
}

bool TOF1D::channel_relevant(int16_t channel) const
{
  return ((channel >= 0) && channels_.relevant(channel));
}

void TOF1D::_push_stats(const Status& status)
{
  if (!this->channel_relevant(status.channel()))
    return;

  if (status.stats().count("native_time"))
    pulse_times_[status.channel()] = status.stats().at("native_time");

  Spectrum::_push_stats(status);
}

void TOF1D::_push_event(const Event& e)
{
  const auto& c = e.channel();

  if (!this->channel_relevant(c)
      || !pulse_times_.count(c)
      || !resolution_)
    return;

  double nsecs =
      (e.timestamp() - TimeStamp(pulse_times_[c], e.timestamp().base()));

  if (nsecs < 0)
    return;

  size_t val = static_cast<size_t>(nsecs / resolution_);

  if (val >= domain_.size())
  {
    size_t oldbound = domain_.size();
    domain_.resize(val+1);

    for (size_t i=oldbound; i <= val; ++i)
      domain_[i] = i * resolution_;
  }

  data_->add({{val}, 1});
  total_count_++;
  recent_count_++;
}
