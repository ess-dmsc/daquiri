#include "Spectrum1DEvents.h"
#include <boost/filesystem.hpp>
#include "dense1d.h"

#include "custom_logger.h"

Spectrum1DEvents::Spectrum1DEvents()
  : SpectrumEventMode()
{
  data_ = std::make_shared<Dense1D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Event mode 1D spectrum", 1);

  SettingMeta app("appearance", SettingType::color);
  app.set_val("description", "Plot appearance");
  base_options.branches.add(Setting(app));

  SettingMeta resm("resolution", SettingType::menu);
  resm.set_flag("preset");
  resm.set_enum(0, "native");
  resm.set_enum(1, "1 bit (2)");
  resm.set_enum(2, "2 bit (4)");
  resm.set_enum(3, "3 bit (8)");
  resm.set_enum(4, "4 bit (16)");
  resm.set_enum(5, "5 bit (32)");
  resm.set_enum(6, "6 bit (64)");
  resm.set_enum(7, "7 bit (128)");
  resm.set_enum(8, "8 bit (256)");
  resm.set_enum(9, "9 bit (512)");
  resm.set_enum(10, "10 bit (1024)");
  resm.set_enum(11, "11 bit (2048)");
  resm.set_enum(12, "12 bit (4096)");
  resm.set_enum(13, "13 bit (8192)");
  resm.set_enum(14, "14 bit (16384)");
  resm.set_enum(15, "15 bit (32768)");
  resm.set_enum(16, "16 bit (65536)");
  Setting res(resm);
  res.set_number(14);
  base_options.branches.add(res);

  SettingMeta cutoff_bin("cutoff_bin", SettingType::integer);
  cutoff_bin.set_val("description", "Hits rejected below minimum energy (after coincidence logic)");
  cutoff_bin.set_val("min", 0);
  cutoff_bin.set_flag("preset");
  base_options.branches.add(cutoff_bin);

  metadata_.overwrite_all_attributes(base_options);
}

bool Spectrum1DEvents::_initialize()
{
  SpectrumEventMode::_initialize();
  bits_ = metadata_.get_attribute("resolution").selection();
  cutoff_bin_ = metadata_.get_attribute("cutoff_bin").get_number();

//  size_t size = pow(2, bits_);
//  if (spectrum_.size() < size)
//    spectrum_.resize(size, PreciseFloat(0));

  this->_recalc_axes();

  return true;
}

void Spectrum1DEvents::_init_from_file(std::string filename)
{
  metadata_.set_attribute(Setting::integer("resolution", bits_));

  pattern_coinc_.resize(1);
  pattern_coinc_.set_gates(std::vector<bool>({true}));

  pattern_anti_.resize(1);
  pattern_anti_.set_gates(std::vector<bool>({false}));

  pattern_add_.resize(1);
  pattern_add_.set_gates(std::vector<bool>({true}));

  total_coincidences_ = total_count_;

  std::string name = boost::filesystem::path(filename).filename().string();
  std::replace( name.begin(), name.end(), '.', '_');

  SpectrumEventMode::_init_from_file(name);
}

void Spectrum1DEvents::_set_detectors(const std::vector<Detector>& dets)
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

void Spectrum1DEvents::_recalc_axes()
{
  data_->set_axis(0, DataAxis(Calibration(), pow(2,bits_), bits_));

  if (data_->dimensions() != metadata_.detectors.size())
    return;

  for (size_t i=0; i < metadata_.detectors.size(); ++i)
  {
    auto det = metadata_.detectors[i];
    CalibID from(det.id(), val_name_, "", bits_);
    CalibID to("", val_name_, "", 0);
    auto calib = det.get_preferred_calibration(from, to);
    data_->set_axis(i, DataAxis(calib, pow(2,bits_), bits_));
  }
}

bool Spectrum1DEvents::event_relevant(const Event& e) const
{
  const auto& c = e.channel();
  return SpectrumEventMode::event_relevant(e) &&
      (e.value(value_idx_[c]).val(bits_) >= cutoff_logic_[c]);
}

void Spectrum1DEvents::bin_event(const Event& e)
{
  uint16_t en = e.value(value_idx_.at(e.channel())).val(bits_);
  if (en < cutoff_bin_)
    return;

  data_->add({{en}, 1});
  total_count_++;
}

void Spectrum1DEvents::add_coincidence(const Coincidence& c)
{
  for (auto &e : c.hits())
    if (pattern_add_.relevant(e.first))
      this->bin_event(e.second);
}
