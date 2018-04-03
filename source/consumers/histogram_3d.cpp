#include "histogram_3d.h"

#include "sparse_map3d.h"
//#include "sparse_matrix3d.h"
//#include "dense_matrix3d.h"

#include "custom_logger.h"

Histogram3D::Histogram3D()
  : Spectrum()
{
  data_ = std::make_shared<SparseMap3D>();
//  data_ = std::make_shared<SparseMatrix3D>();
//  data_ = std::make_shared<DenseMatrix3D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Event-based 3D spectrum");

  SettingMeta x_name("x_name", SettingType::text);
  x_name.set_flag("preset");
  x_name.set_flag("event_value");
  x_name.set_val("description", "Name of event value for x coordinate");
  base_options.branches.add(x_name);

  SettingMeta y_name("y_name", SettingType::text);
  y_name.set_flag("preset");
  y_name.set_flag("event_value");
  y_name.set_val("description", "Name of event value for y coordinate");
  base_options.branches.add(y_name);

  SettingMeta z_name("z_name", SettingType::text);
  z_name.set_flag("preset");
  z_name.set_flag("event_value");
  z_name.set_val("description", "Name of event value for z coordinate");
  base_options.branches.add(z_name);

  SettingMeta ds("downsample", SettingType::integer, "Downsample bins by");
  ds.set_val("units", "bits");
  ds.set_flag("preset");
  ds.set_val("min", 0);
  ds.set_val("max", 31);
  base_options.branches.add(ds);

  base_options.branches.add(filters_.settings());

  metadata_.overwrite_all_attributes(base_options);
}

void Histogram3D::_apply_attributes()
{
  Spectrum::_apply_attributes();

  x_name_ = metadata_.get_attribute("x_name").get_text();
  y_name_ = metadata_.get_attribute("y_name").get_text();
  z_name_ = metadata_.get_attribute("z_name").get_text();
  downsample_ = metadata_.get_attribute("downsample").get_number();

  filters_.settings(metadata_.get_attribute("filters"));
  metadata_.replace_attribute(filters_.settings());
}

void Histogram3D::_init_from_file()
{
  metadata_.set_attribute(Setting::integer("downsample", downsample_));
  metadata_.set_attribute(Setting::text("x_name", "value1"));
  metadata_.set_attribute(Setting::text("y_name", "value2"));
  metadata_.set_attribute(Setting::text("z_name", "value3"));

  Spectrum::_init_from_file();
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

  auto calib0 = det0.get_calibration({x_name_, det0.id()}, {x_name_});
  data_->set_axis(0, DataAxis(calib0, downsample_));

  auto calib1 = det1.get_calibration({y_name_, det1.id()}, {y_name_});
  data_->set_axis(1, DataAxis(calib1, downsample_));

  auto calib2 = det2.get_calibration({z_name_, det2.id()}, {z_name_});
  data_->set_axis(2, DataAxis(calib2, downsample_));

  data_->recalc_axes();
}

void Histogram3D::_push_stats_pre(const Spill &spill)
{
  if (this->_accept_spill(spill))
  {
    x_idx_ = spill.event_model.name_to_val.at(x_name_);
    y_idx_ = spill.event_model.name_to_val.at(y_name_);
    z_idx_ = spill.event_model.name_to_val.at(z_name_);
    filters_.configure(spill);
    Spectrum::_push_stats_pre(spill);
  }
}

void Histogram3D::_flush()
{
  Spectrum::_flush();
}

void Histogram3D::_push_event(const Event& event)
{
  if (!filters_.accept(event))
    return;

  if (downsample_)
  {
    coords_[0] = (event.value(x_idx_) >> downsample_);
    coords_[1] = (event.value(y_idx_) >> downsample_);
    coords_[2] = (event.value(z_idx_) >> downsample_);
  }
  else
  {
    coords_[0] = event.value(x_idx_);
    coords_[1] = event.value(y_idx_);
    coords_[2] = event.value(z_idx_);
  }

  data_->add_one(coords_);
  total_count_++;
  recent_count_++;
}

bool Histogram3D::_accept_spill(const Spill& spill)
{
  return (Spectrum::_accept_spill(spill)
          && spill.event_model.name_to_val.count(x_name_)
          && spill.event_model.name_to_val.count(y_name_)
          && spill.event_model.name_to_val.count(z_name_)
          );
}

bool Histogram3D::_accept_events(const Spill& /*spill*/)
{
  return (x_idx_ >= 0) && (y_idx_ >= 0) && (z_idx_ >= 0);
}
