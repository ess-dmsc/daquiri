#include "tof_1d.h"
#include <boost/filesystem.hpp>
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

void TOF1D::_init_from_file(std::string filename)
{
//  metadata_.set_attribute(Setting::integer("resolution", bits_));

  channels_.resize(1);
  channels_.set_gates(std::vector<bool>({true}));
  metadata_.set_attribute(Setting("channels", channels_));

  std::string name = boost::filesystem::path(filename).filename().string();
  std::replace( name.begin(), name.end(), '.', '_');

  Spectrum::_init_from_file(name);
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
  data_->set_axis(0, DataAxis(Calibration(), 0));
  data_->recalc_axes(0);
}

bool TOF1D::channel_relevant(int16_t channel) const
{
  return ((channel >= 0) && channels_.relevant(channel));
}

void TOF1D::_push_stats(const Status& newBlock)
{
  if (!this->channel_relevant(newBlock.channel()))
    return;

  Spectrum::_push_stats(newBlock);

  if (newBlock.stats().count("native_time"))
    pulse_times_[newBlock.channel()] = newBlock.stats().at("native_time");
}

void TOF1D::_push_event(const Event& e)
{
  const auto& c = e.channel();

  if (!this->channel_relevant(c)
      || !pulse_times_.count(c))
    return;

  double v = e.timestamp() -
      TimeStamp(pulse_times_[c], e.timestamp().base());

  if (v < 0)
    return;

  data_->add({{v / resolution_}, 1});
  total_count_++;
}
