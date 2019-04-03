#include <consumers/histogram_1d.h>
#include <consumers/dataspaces/dense1d.h>

#include <core/util/logger.h>

namespace DAQuiri {

Histogram1D::Histogram1D()
    : Spectrum()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "1D Histogram");

  SettingMeta app("appearance", SettingType::text, "Appearance");
  app.set_flag("color");
  base_options.branches.add(Setting(app));

  base_options.branches.add(value_latch_.settings(-1, "Value to bin"));

  metadata_.overwrite_all_attributes(base_options);
}

void Histogram1D::_apply_attributes()
{
  Spectrum::_apply_attributes();

  value_latch_.settings(metadata_.get_attribute("value_latch"));
  metadata_.replace_attribute(value_latch_.settings(-1, "Value to bin"));

  this->_recalc_axes();
}

void Histogram1D::_recalc_axes()
{
  Detector det;
  if (data_->dimensions() == metadata_.detectors.size())
    det = metadata_.detectors[0];

  auto calib = det.get_calibration({value_latch_.value_id, det.id()}, {value_latch_.value_id});
  data_->set_axis(0, DataAxis(calib, value_latch_.downsample));

  data_->recalc_axes();
}

bool Histogram1D::_accept_spill(const Spill& spill)
{
  return (Spectrum::_accept_spill(spill) &&
      value_latch_.has_declared_value(spill));
}

void Histogram1D::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;
  value_latch_.configure(spill);
  Spectrum::_push_stats_pre(spill);
}

bool Histogram1D::_accept_events(const Spill& /*spill*/)
{
  return value_latch_.valid();
}

void Histogram1D::_push_event(const Event& event)
{
  if (!filters_.accept(event))
    return;

  value_latch_.extract(coords_[0], event);
  data_->add_one(coords_);
}

}