#include "Spectrum1DEvents.h"
#include <boost/filesystem.hpp>

#include "custom_logger.h"


Spectrum1DEvents::Spectrum1DEvents()
{
  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Event mode 1D spectrum", 1);

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
  Spectrum1D::_initialize();

  cutoff_bin_ = metadata_.get_attribute("cutoff_bin").get_number();

  return true;
}

void Spectrum1DEvents::_init_from_file(std::string filename)
{
  pattern_coinc_.resize(1);
  pattern_coinc_.set_gates(std::vector<bool>({true}));

  pattern_anti_.resize(1);
  pattern_anti_.set_gates(std::vector<bool>({false}));

  pattern_add_.resize(1);
  pattern_add_.set_gates(std::vector<bool>({true}));

  total_coincidences_ = total_count_;

  std::string name = boost::filesystem::path(filename).filename().string();
  std::replace( name.begin(), name.end(), '.', '_');

  Spectrum1D::_init_from_file(name);
  SpectrumEventMode::_init_from_file(name);
}

void Spectrum1DEvents::_recalc_axes()
{
  Spectrum1D::_recalc_axes();

  if (axes_.size() != metadata_.detectors.size())
    return;

  for (size_t i=0; i < metadata_.detectors.size(); ++i)
  {
    auto det = metadata_.detectors[i];
    CalibID from(det.id(), val_name_, "", bits_);
    CalibID to("", val_name_, "", 0);
    auto calib = det.get_preferred_calibration(from, to);
    DBG << "Adopted calibration " << calib.debug();
    axes_[i] = DataAxis(calib, pow(2,bits_), bits_);
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
  if ((en < cutoff_bin_) || (en >= spectrum_.size()))
    return;

  ++spectrum_[en];
  total_count_++;

  if (en > maxchan_)
    maxchan_ = en;
}

void Spectrum1DEvents::add_coincidence(const Coincidence& c)
{
  for (auto &e : c.hits())
    if (pattern_add_.relevant(e.first))
      this->bin_event(e.second);
}
