#include "spectrum_events_2D.h"
#include <boost/filesystem.hpp>

#include "custom_logger.h"

//#include "consumer_factory.h"
//static ConsumerRegistrar<Spectrum2DEvent> registrar("1DEvent");

Spectrum2DEvent::Spectrum2DEvent()
{
  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata("2DEvent", "Event mode 2D spectrum", 1);
  metadata_.overwrite_all_attributes(base_options);
}

bool Spectrum2DEvent::_initialize()
{
  SpectrumEventMode::_initialize();
  Spectrum2D::_initialize();

  int adds = 0;
  std::vector<bool> gts = pattern_add_.gates();
  for (size_t i=0; i < gts.size(); ++i)
    if (gts[i])
      adds++;

  if (adds != 2)
  {
    WARN << "<Spectrum2DEvent> Cannot initialize. Add pattern must have 2 selected channels.";
    return false;
  }

  pattern_.resize(2, 0);
  adds = 0;
  for (size_t i=0; i < gts.size(); ++i)
  {
    if (gts[i])
    {
      pattern_[adds] = i;
      adds++;
    }
  }

  return true;
}

void Spectrum2DEvent::_init_from_file(std::string filename)
{
  pattern_coinc_.resize(2);
  pattern_coinc_.set_gates(std::vector<bool>({true, true}));

  pattern_anti_.resize(2);
  pattern_anti_.set_gates(std::vector<bool>({false, false}));

  pattern_add_.resize(2);
  pattern_add_.set_gates(std::vector<bool>({true, true}));

  total_coincidences_ = total_count_;

  std::string name = boost::filesystem::path(filename).filename().string();
  std::replace( name.begin(), name.end(), '.', '_');

  Spectrum2D::_init_from_file(name);
  SpectrumEventMode::_init_from_file(name);
}

void Spectrum2DEvent::_recalc_axes()
{
  Spectrum2D::_recalc_axes();

  if (axes_.size() != metadata_.detectors.size())
    return;

  for (size_t i=0; i < metadata_.detectors.size(); ++i)
  {
    auto det = metadata_.detectors[i];
    CalibID from(det.id(), val_name_, "", bits_);
    CalibID to("", val_name_, "", 0);
    auto calib = det.get_preferred_calibration(from, to);
    axes_[i] = DataAxis(calib, pow(2,bits_), bits_);
  }
}

bool Spectrum2DEvent::event_relevant(const Event& e) const
{
  const auto& c = e.channel();
  return SpectrumEventMode::event_relevant(e) &&
      (e.value(value_idx_[c]).val(bits_) >= cutoff_logic_[c]);
}

void Spectrum2DEvent::add_coincidence(const Coincidence& c)
{
  std::list<uint16_t> l0, l1;
  for (auto &e : c.hits())
  {
    if (e.first == pattern_[0])
      l0.push_back(e.second.value(value_idx_.at(pattern_[0])).val(bits_));
    else if (e.first == pattern_[1])
      l1.push_back(e.second.value(value_idx_.at(pattern_[1])).val(bits_));
  }

  if (l0.empty())
    l0.push_back(0);
  if (l1.empty())
    l1.push_back(0);

  for (const auto& ll0 : l0)
    for (const auto& ll1 : l1)
      Spectrum2D::bin_pair(ll0, ll1, 1);
}
