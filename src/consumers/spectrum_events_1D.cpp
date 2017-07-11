#include "spectrum_events_1D.h"
#include <boost/filesystem.hpp>

//#include "consumer_factory.h"
//static ConsumerRegistrar<Spectrum1DEvent> registrar("1DEvent");

Spectrum1DEvent::Spectrum1DEvent()
{
  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata("1DEvent", "Event mode 1D spectrum", 1);

  SettingMeta cutoff_bin("cutoff_bin", SettingType::integer);
  cutoff_bin.set_val("description", "Hits rejected below minimum energy (after coincidence logic)");
  cutoff_bin.set_val("min", 0);
  cutoff_bin.set_flag("preset");
  base_options.branches.add(cutoff_bin);

  metadata_.overwrite_all_attributes(base_options);
}

bool Spectrum1DEvent::_initialize()
{
  SpectrumEventMode::_initialize();
  Spectrum1D::_initialize();

  cutoff_bin_ = metadata_.get_attribute("cutoff_bin").get_number();

  return true;
}

void Spectrum1DEvent::_init_from_file(std::string filename)
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

void Spectrum1DEvent::bin_event(const Event& newHit)
{
  uint16_t en = newHit.value(energy_idx_.at(newHit.channel())).val(bits_);
  if (en < cutoff_bin_)
    return;

  ++spectrum_[en];
  total_count_++;

  if (en > maxchan_)
    maxchan_ = en;
}

bool Spectrum1DEvent::event_relevant(const Event& e) const
{
  return ((e.channel() < static_cast<int16_t>(energy_idx_.size())) &&
          (e.value(energy_idx_.at(e.channel())).val(bits_) >= cutoff_logic_[e.channel()]));

}

void Spectrum1DEvent::add_coincidence(const Coincidence& c)
{
  for (auto &e : c.hits())
    if (pattern_add_.relevant(e.first))
      this->bin_event(e.second);
}
