#include "Image2D.h"
#include <boost/filesystem.hpp>
#include "sparse2d.h"

#include "custom_logger.h"

Image2D::Image2D()
  : Spectrum()
{
  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Values-based 2D image", 2);

  SettingMeta x_name("x_name", SettingType::text);
  x_name.set_flag("preset");
  x_name.set_val("description", "Name of event value for x coordinate");
  base_options.branches.add(x_name);

  SettingMeta y_name("y_name", SettingType::text);
  y_name.set_flag("preset");
  y_name.set_val("description", "Name of event value for y coordinate");
  base_options.branches.add(y_name);

  SettingMeta ds("downsample", SettingType::integer);
  ds.set_flag("preset");
  ds.set_val("min", 0);
  ds.set_val("max", 15);
  base_options.branches.add(ds);

  SettingMeta pattern_add("pattern_add", SettingType::pattern);
  pattern_add.set_flag("preset");
  pattern_add.set_val("description", "Add pattern");
  pattern_add.set_val("chans", 1);
  base_options.branches.add(pattern_add);

  metadata_.overwrite_all_attributes(base_options);
}

bool Image2D::_initialize()
{
  Spectrum::_initialize();

  x_name_ = metadata_.get_attribute("x_name").get_text();
  y_name_ = metadata_.get_attribute("y_name").get_text();
  pattern_add_ = metadata_.get_attribute("pattern_add").pattern();
  downsample_ = metadata_.get_attribute("downsample").get_number();

  return true;
}

void Image2D::_init_from_file(std::string filename)
{
  metadata_.set_attribute(Setting("pattern_add", pattern_add_));
  metadata_.set_attribute(Setting::integer("downsample", downsample_));
  metadata_.set_attribute(Setting::text("x_name", "value1"));
  metadata_.set_attribute(Setting::text("y_name", "value2"));

  std::string name = boost::filesystem::path(filename).filename().string();
  std::replace( name.begin(), name.end(), '.', '_');

  Spectrum::_init_from_file(name);
}

void Image2D::_set_detectors(const std::vector<Detector>& dets)
{
  metadata_.detectors.resize(metadata_.dimensions(), Detector());

  if (dets.size() == metadata_.dimensions())
    metadata_.detectors = dets;

  if (dets.size() >= metadata_.dimensions())
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

void Image2D::_recalc_axes()
{
  uint16_t bits_ = 16;
  data_->set_axis(0, DataAxis(Calibration(), pow(2,bits_), bits_));
  data_->set_axis(1, DataAxis(Calibration(), pow(2,bits_), bits_));

  if (data_->dimensions() != metadata_.detectors.size())
    return;

  for (size_t i=0; i < metadata_.detectors.size(); ++i)
  {
    auto det = metadata_.detectors[i];
    std::string valname = (i == 0) ? x_name_ : y_name_;
    CalibID from(det.id(), valname, "", bits_);
    CalibID to("", valname, "", 0);
    auto calib = det.get_preferred_calibration(from, to);
    data_->set_axis(i, DataAxis(calib, pow(2,bits_), bits_));
  }
}

void Image2D::_push_stats(const Status& manifest)
{
  if (!this->channel_relevant(manifest.channel()))
    return;

  Spectrum::_push_stats(manifest);

  if (manifest.channel() >= static_cast<int16_t>(x_idx_.size()))
  {
    x_idx_.resize(manifest.channel() + 1, -1);
    y_idx_.resize(manifest.channel() + 1, -1);
  }
  if (manifest.event_model().name_to_val.count(x_name_))
    x_idx_[manifest.channel()] = manifest.event_model().name_to_val.at(x_name_);
  if (manifest.event_model().name_to_val.count(y_name_))
    y_idx_[manifest.channel()] = manifest.event_model().name_to_val.at(y_name_);
}

void Image2D::_flush()
{
  Spectrum::_flush();
}

void Image2D::_push_event(const Event& e)
{
  uint16_t bits_ = 16;

  if (!this->event_relevant(e))
    return;
  const auto& c = e.channel();
  uint16_t x = e.value(x_idx_.at(c)).val(bits_);
  uint16_t y = e.value(y_idx_.at(c)).val(bits_);
  data_->add({{x,y},1});
}

bool Image2D::channel_relevant(int16_t channel) const
{
  return ((channel >= 0) &&
          pattern_add_.relevant(channel)
          );
}

bool Image2D::event_relevant(const Event& e) const
{
  const auto& c = e.channel();
  return (this->channel_relevant(c) &&
          value_relevant(c, x_idx_) &&
          value_relevant(c, y_idx_)
          );
}
