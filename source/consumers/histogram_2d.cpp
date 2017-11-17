#include "histogram_2d.h"

#include "sparse_map2d.h"
#include "sparse_matrix2d.h"
#include "dense_matrix2d.h"

#include "custom_logger.h"

#define kDimensions 2

Histogram2D::Histogram2D()
  : Spectrum()
{
//  data_ = std::make_shared<SparseMap2D>();
  data_ = std::make_shared<SparseMatrix2D>();
//  data_ = std::make_shared<DenseMatrix2D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Event-based 2D spectrum");

  SettingMeta x_name("x_name", SettingType::text);
  x_name.set_flag("preset");
  x_name.set_val("description", "Name of event value for x coordinate");
  base_options.branches.add(x_name);

  SettingMeta y_name("y_name", SettingType::text);
  y_name.set_flag("preset");
  y_name.set_val("description", "Name of event value for y coordinate");
  base_options.branches.add(y_name);

  SettingMeta ds("downsample", SettingType::integer, "Downsample x&y by");
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

bool Histogram2D::_initialize()
{
  Spectrum::_initialize();

  x_name_ = metadata_.get_attribute("x_name").get_text();
  y_name_ = metadata_.get_attribute("y_name").get_text();
  add_channels_ = metadata_.get_attribute("add_channels").pattern();
  downsample_ = metadata_.get_attribute("downsample").get_number();

  return true;
}

void Histogram2D::_init_from_file()
{
  metadata_.set_attribute(Setting("add_channels", add_channels_));
  metadata_.set_attribute(Setting::integer("downsample", downsample_));
  metadata_.set_attribute(Setting::text("x_name", "value1"));
  metadata_.set_attribute(Setting::text("y_name", "value2"));

  Spectrum::_init_from_file();
}

void Histogram2D::_set_detectors(const std::vector<Detector>& dets)
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

void Histogram2D::_recalc_axes()
{
  Detector det0, det1;
  if (data_->dimensions() == metadata_.detectors.size())
  {
    det0 = metadata_.detectors[0];
    det1 = metadata_.detectors[1];
  }

  auto calib0 = det0.get_calibration({x_name_, det0.id()}, {x_name_});
  data_->set_axis(0, DataAxis(calib0, downsample_));

  auto calib1 = det1.get_calibration({y_name_, det1.id()}, {y_name_});
  data_->set_axis(1, DataAxis(calib1, downsample_));

  data_->recalc_axes();
}

void Histogram2D::_push_stats_pre(const Status& manifest)
{
  if (!this->channel_relevant(manifest.channel()))
    return;

  if (manifest.channel() >= static_cast<int16_t>(x_idx_.size()))
  {
    x_idx_.resize(manifest.channel() + 1, -1);
    y_idx_.resize(manifest.channel() + 1, -1);
  }
  if (manifest.event_model().name_to_val.count(x_name_))
    x_idx_[manifest.channel()] = manifest.event_model().name_to_val.at(x_name_);
  if (manifest.event_model().name_to_val.count(y_name_))
    y_idx_[manifest.channel()] = manifest.event_model().name_to_val.at(y_name_);

  Spectrum::_push_stats_pre(manifest);
}

void Histogram2D::_flush()
{
  Spectrum::_flush();
}

void Histogram2D::_push_event(const Event& e)
{
  if (!this->event_relevant(e))
    return;
  const auto& c = e.channel();

  if (downsample_)
  {
    coords_[0] = (e.value(x_idx_[c]) >> downsample_);
    coords_[1] = (e.value(y_idx_[c]) >> downsample_);
  }
  else
  {
    coords_[0] = e.value(x_idx_[c]);
    coords_[1] = e.value(y_idx_[c]);
  }

  data_->add_one(coords_);
  total_count_++;
  recent_count_++;
}

bool Histogram2D::channel_relevant(int16_t channel) const
{
  return ((channel >= 0) &&
          add_channels_.relevant(channel)
          );
}

bool Histogram2D::event_relevant(const Event& e) const
{
  const auto& c = e.channel();
  return (this->channel_relevant(c) &&
          value_relevant(c, x_idx_) &&
          value_relevant(c, y_idx_)
          );
}
