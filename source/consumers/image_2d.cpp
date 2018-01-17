#include "image_2d.h"

#include "sparse_map2d.h"
#include "sparse_matrix2d.h"

#include "custom_logger.h"

#define kDimensions 2

Image2D::Image2D()
  : Spectrum()
{
//  data_ = std::make_shared<SparseMap2D>();
  data_ = std::make_shared<SparseMatrix2D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Values-based 2D image");

  SettingMeta app("appearance", SettingType::text, "Appearance");
  app.set_flag("gradient-name");
  base_options.branches.add(Setting(app));

  SettingMeta x_name("x_name", SettingType::text);
  x_name.set_flag("preset");
  x_name.set_val("description", "Name of event value for x coordinate");
  base_options.branches.add(x_name);

  SettingMeta y_name("y_name", SettingType::text);
  y_name.set_flag("preset");
  y_name.set_val("description", "Name of event value for y coordinate");
  base_options.branches.add(y_name);

  SettingMeta v_name("val_name", SettingType::text);
  v_name.set_flag("preset");
  v_name.set_val("description", "Name of event value for intensity");
  base_options.branches.add(v_name);

  SettingMeta ds("downsample", SettingType::integer, "Downsample x&y by");
  ds.set_val("units", "bits");
  ds.set_flag("preset");
  ds.set_val("min", 0);
  ds.set_val("max", 31);
  base_options.branches.add(ds);

  metadata_.overwrite_all_attributes(base_options);
}

bool Image2D::_initialize()
{
  Spectrum::_initialize();

  x_name_ = metadata_.get_attribute("x_name").get_text();
  y_name_ = metadata_.get_attribute("y_name").get_text();
  val_name_ = metadata_.get_attribute("val_name").get_text();
  downsample_ = metadata_.get_attribute("downsample").get_number();

  return true;
}

void Image2D::_init_from_file()
{
  metadata_.set_attribute(Setting::integer("downsample", downsample_));
  metadata_.set_attribute(Setting::text("x_name", "x"));
  metadata_.set_attribute(Setting::text("y_name", "y"));
  metadata_.set_attribute(Setting::text("val_name", "val"));

  Spectrum::_init_from_file();
}

void Image2D::_set_detectors(const std::vector<Detector>& dets)
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

void Image2D::_recalc_axes()
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

bool Image2D::_accept_spill(const Spill& spill)
{
  return (Spectrum::_accept_spill(spill)
          && spill.event_model.name_to_val.count(x_name_)
          && spill.event_model.name_to_val.count(y_name_)
          && spill.event_model.name_to_val.count(val_name_)
          );
}

bool Image2D::_accept_events(const Spill &spill)
{
  return (x_idx_ >= 0) && (y_idx_ >= 0) && (val_idx_ >= 0);
}

void Image2D::_push_stats_pre(const Spill &spill)
{
  if (this->_accept_spill(spill))
  {
    x_idx_ = spill.event_model.name_to_val.at(x_name_);
    y_idx_ = spill.event_model.name_to_val.at(y_name_);
    val_idx_ = spill.event_model.name_to_val.at(val_name_);
    Spectrum::_push_stats_pre(spill);
  }
}

void Image2D::_flush()
{
  Spectrum::_flush();
}

void Image2D::_push_event(const Event& e)
{
  if (downsample_)
  {
    entry_.first[0] = (e.value(x_idx_) >> downsample_);
    entry_.first[1] = (e.value(y_idx_) >> downsample_);
  }
  else
  {
    entry_.first[0] = e.value(x_idx_);
    entry_.first[1] = e.value(y_idx_);
  }

  entry_.second = e.value(val_idx_);
  data_->add(entry_);
  total_count_++;  //not += ?
  recent_count_++; //not += ?
}
