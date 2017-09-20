#include "coincidence_1d.h"
#include "dense1d.h"

#include "custom_logger.h"

#define kDimensions 1

Coincidence1D::Coincidence1D()
  : CoincidenceConsumer()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Event mode 1D spectrum");

  SettingMeta app("appearance", SettingType::color);
  app.set_val("description", "Plot appearance");
  base_options.branches.add(Setting(app));

  SettingMeta ds("downsample", SettingType::integer);
  ds.set_val("units", "bits");
  ds.set_flag("preset");
  ds.set_val("min", 0);
  ds.set_val("max", 31);
  base_options.branches.add(ds);

  SettingMeta cutoff_bin("cutoff_bin", SettingType::integer);
  cutoff_bin.set_val("description", "Hits rejected below minimum energy (after coincidence logic)");
  cutoff_bin.set_val("min", 0);
  cutoff_bin.set_flag("preset");
  base_options.branches.add(cutoff_bin);

  metadata_.overwrite_all_attributes(base_options);
}

bool Coincidence1D::_initialize()
{
  CoincidenceConsumer::_initialize();
  downsample_ = metadata_.get_attribute("downsample").get_number();
  cutoff_bin_ = metadata_.get_attribute("cutoff_bin").get_number();

  this->_recalc_axes();
  return true;
}

void Coincidence1D::_init_from_file()
{
  metadata_.set_attribute(Setting::integer("downsample", downsample_));

  pattern_coinc_.resize(1);
  pattern_coinc_.set_gates(std::vector<bool>({true}));

  pattern_anti_.resize(1);
  pattern_anti_.set_gates(std::vector<bool>({false}));

  add_channels_.resize(1);
  add_channels_.set_gates(std::vector<bool>({true}));

  total_coincidences_ = total_count_;

  CoincidenceConsumer::_init_from_file();
}

void Coincidence1D::_set_detectors(const std::vector<Detector>& dets)
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

void Coincidence1D::_recalc_axes()
{
  data_->set_axis(0, DataAxis(Calibration(), 0));

  if (data_->dimensions() != metadata_.detectors.size())
    return;

  for (size_t i=0; i < metadata_.detectors.size(); ++i)
  {
    auto det = metadata_.detectors[i];
    CalibID from(det.id(), val_name_, "", 0);
    CalibID to("", val_name_, "", 0);
    auto calib = det.get_preferred_calibration(from, to);
    data_->set_axis(i, DataAxis(calib, 0));
  }

  data_->recalc_axes(0);
}

bool Coincidence1D::event_relevant(const Event& e) const
{
  const auto& c = e.channel();
  return CoincidenceConsumer::event_relevant(e) &&
      (e.value(value_idx_[c]) >= cutoff_logic_[c]);
}

void Coincidence1D::bin_event(const Event& e)
{
  uint16_t en = e.value(value_idx_[e.channel()]);
  if (en < cutoff_bin_)
    return;

  data_->add({{en}, 1});
  total_count_++;
  recent_count_++;
}

void Coincidence1D::add_coincidence(const Coincidence& c)
{
  for (auto &e : c.hits())
    if (add_channels_.relevant(e.first))
      this->bin_event(e.second);
}
