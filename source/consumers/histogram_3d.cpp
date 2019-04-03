#include <consumers/histogram_3d.h>

#include <consumers/dataspaces/sparse_map3d.h>
//#include <consumers/dataspaces/sparse_matrix3d.h>
//#include <consumers/dataspaces/dense_matrix3d.h>

#include <core/util/logger.h>

namespace DAQuiri {

Histogram3D::Histogram3D()
    : Spectrum()
{
  data_ = std::make_shared<SparseMap3D>();
//  data_ = std::make_shared<SparseMatrix3D>();
//  data_ = std::make_shared<DenseMatrix3D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Event-based 3D spectrum");

  base_options.branches.add_a(value_latch_x_.settings(0, "X value"));
  base_options.branches.add_a(value_latch_y_.settings(1, "Y value"));
  base_options.branches.add_a(value_latch_z_.settings(2, "Z value"));

  metadata_.overwrite_all_attributes(base_options);
}

void Histogram3D::_apply_attributes()
{
  Spectrum::_apply_attributes();

  value_latch_x_.settings(metadata_.get_attribute(value_latch_x_.settings(0, "X value")));
  metadata_.replace_attribute(value_latch_x_.settings(0, "X value"));

  value_latch_y_.settings(metadata_.get_attribute(value_latch_y_.settings(1, "Y value")));
  metadata_.replace_attribute(value_latch_y_.settings(1, "Y value"));

  value_latch_z_.settings(metadata_.get_attribute(value_latch_z_.settings(2, "Z value")));
  metadata_.replace_attribute(value_latch_z_.settings(2, "Z value"));
}

void Histogram3D::_recalc_axes()
{
  Detector det0, det1, det2;
  if (data_->dimensions() == metadata_.detectors.size())
  {
    det0 = metadata_.detectors[0];
    det1 = metadata_.detectors[1];
    det2 = metadata_.detectors[2];
  }

  auto calib0 = det0.get_calibration({value_latch_x_.value_id, det0.id()}, {value_latch_x_.value_id});
  data_->set_axis(0, DataAxis(calib0, value_latch_x_.downsample));

  auto calib1 = det1.get_calibration({value_latch_y_.value_id, det1.id()}, {value_latch_y_.value_id});
  data_->set_axis(1, DataAxis(calib1, value_latch_y_.downsample));

  auto calib2 = det2.get_calibration({value_latch_z_.value_id, det2.id()}, {value_latch_z_.value_id});
  data_->set_axis(2, DataAxis(calib2, value_latch_z_.downsample));

  data_->recalc_axes();
}

bool Histogram3D::_accept_spill(const Spill& spill)
{
  return (Spectrum::_accept_spill(spill) &&
      value_latch_x_.has_declared_value(spill) &&
      value_latch_y_.has_declared_value(spill) &&
      value_latch_z_.has_declared_value(spill));
}


void Histogram3D::_push_stats_pre(const Spill& spill)
{
  if (!this->_accept_spill(spill))
    return;
  value_latch_x_.configure(spill);
  value_latch_y_.configure(spill);
  value_latch_z_.configure(spill);
  Spectrum::_push_stats_pre(spill);
}

bool Histogram3D::_accept_events(const Spill& /*spill*/)
{
  return value_latch_x_.valid() && value_latch_y_.valid() && value_latch_z_.valid();
}

void Histogram3D::_push_event(const Event& event)
{
  if (!filters_.accept(event))
    return;

  value_latch_x_.extract(coords_[0], event);
  value_latch_y_.extract(coords_[1], event);
  value_latch_z_.extract(coords_[2], event);
  
  data_->add_one(coords_);
}

}
