#include "histogram_1d.h"
#include "dense1d.h"

#include "custom_logger.h"

#define kDimensions 1

Histogram1D::Histogram1D()
  : Spectrum()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "1D Histogram");

  SettingMeta app("appearance", SettingType::color, "Plot appearance");
  base_options.branches.add(Setting(app));

  SettingMeta ds("downsample", SettingType::integer, "Downsample by");
  ds.set_val("units", "bits");
  ds.set_flag("preset");
  ds.set_val("min", 0);
  ds.set_val("max", 31);
  base_options.branches.add(ds);

  SettingMeta cutoff_bin("cutoff", SettingType::integer, "Hits rejected below minimum value");
  cutoff_bin.set_val("min", 0);
  cutoff_bin.set_flag("preset");
  base_options.branches.add(cutoff_bin);

  SettingMeta val_name("value_name", SettingType::text, "Name of event value to bin");
  val_name.set_flag("preset");
  base_options.branches.add(val_name);

  metadata_.overwrite_all_attributes(base_options);
}

bool Histogram1D::_initialize()
{
  Spectrum::_initialize();
  downsample_ = metadata_.get_attribute("downsample").get_number();
  cutoff_bin_ = metadata_.get_attribute("cutoff").get_number();
  val_name_ = metadata_.get_attribute("value_name").get_text();

  this->_recalc_axes();
  return true;
}

void Histogram1D::_init_from_file()
{
  metadata_.set_attribute(Setting::integer("downsample", downsample_));
  metadata_.set_attribute(Setting::integer("cutoff", cutoff_bin_));
  Spectrum::_init_from_file();
}

void Histogram1D::_set_detectors(const std::vector<Detector>& dets)
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

void Histogram1D::_recalc_axes()
{
  Detector det;
  if (data_->dimensions() == metadata_.detectors.size())
    det = metadata_.detectors[0];

  auto calib = det.get_calibration({val_name_, det.id()}, {val_name_});
  data_->set_axis(0, DataAxis(calib, downsample_));

  data_->recalc_axes();
}

bool Histogram1D::_accept_spill(const Spill& spill)
{
  return (Spectrum::_accept_spill(spill)
          &&
          spill.event_model.name_to_val.count(val_name_));
}

bool Histogram1D::_accept_events()
{
  return (value_idx_ >= 0);
}

void Histogram1D::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;

  value_idx_ = spill.event_model.name_to_val.at(val_name_);

  Spectrum::_push_stats_pre(spill);
}

void Histogram1D::_push_event(const Event& event)
{
  if (downsample_)
    coords_[0] = (event.value(value_idx_) >> downsample_);
  else
    coords_[0] = event.value(value_idx_);

  if (coords_[0] < cutoff_bin_)
    return;

  data_->add_one(coords_);
  total_count_++;
  recent_count_++;
}
