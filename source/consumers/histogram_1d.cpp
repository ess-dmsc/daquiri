#include "histogram_1d.h"
#include "dense1d.h"

#include "custom_logger.h"

Histogram1D::Histogram1D()
  : Spectrum()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "1D Histogram");

  SettingMeta app("appearance", SettingType::text, "Appearance");
  app.set_flag("color");
  base_options.branches.add(Setting(app));

  SettingMeta ds("downsample", SettingType::integer, "Downsample by");
  ds.set_val("units", "bits");
  ds.set_flag("preset");
  ds.set_val("min", 0);
  ds.set_val("max", 31);
  base_options.branches.add(ds);

  SettingMeta val_name("value_name", SettingType::text, "Name of event value to bin");
  val_name.set_flag("preset");
  val_name.set_flag("event_value");
  base_options.branches.add(val_name);

  base_options.branches.add(filters_.settings());

  metadata_.overwrite_all_attributes(base_options);
}

void Histogram1D::_apply_attributes()
{
  Spectrum::_apply_attributes();
  downsample_ = metadata_.get_attribute("downsample").get_number();
  val_name_ = metadata_.get_attribute("value_name").get_text();

  filters_.settings(metadata_.get_attribute("filters"));
  metadata_.replace_attribute(filters_.settings());

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

bool Histogram1D::_accept_events(const Spill &spill)
{
  return (value_idx_ >= 0);
}

void Histogram1D::_push_stats_pre(const Spill& spill)
{
  if (this->_accept_spill(spill))
  {
    value_idx_ = spill.event_model.name_to_val.at(val_name_);
    filters_.configure(spill);
    Spectrum::_push_stats_pre(spill);
  }
}

void Histogram1D::_push_event(const Event& event)
{
  if (!filters_.accept(event))
    return;

  if (downsample_)
    coords_[0] = (event.value(value_idx_) >> downsample_);
  else
    coords_[0] = event.value(value_idx_);

  data_->add_one(coords_);
  total_count_++;
  recent_count_++;
}
