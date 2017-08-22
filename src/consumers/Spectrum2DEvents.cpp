#include "Spectrum2DEvents.h"
#include <boost/filesystem.hpp>
#include "sparse2d.h"

#include "custom_logger.h"

Spectrum2DEvents::Spectrum2DEvents()
  : SpectrumEventMode()
{
  data_ = std::make_shared<Sparse2D>();

  Setting base_options = metadata_.attributes();
  metadata_ = ConsumerMetadata(my_type(), "Event mode 2D spectrum", 2);

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

  SettingMeta sym("symmetric", SettingType::boolean);
  sym.set_val("description", "Matrix is symmetric");
  sym.set_flag("readonly");
  base_options.branches.add(sym);

  metadata_.overwrite_all_attributes(base_options);
}

bool Spectrum2DEvents::_initialize()
{
  SpectrumEventMode::_initialize();
  bits_ = metadata_.get_attribute("resolution").selection();

  int adds = 0;
  std::vector<bool> gts = pattern_add_.gates();
  for (size_t i=0; i < gts.size(); ++i)
    if (gts[i])
      adds++;

  if (adds != 2)
  {
    WARN << "<Spectrum2DEvents> Cannot initialize. Add pattern must have 2 selected channels.";
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

void Spectrum2DEvents::_init_from_file(std::string filename)
{
  metadata_.set_attribute(Setting::integer("resolution", bits_));

  pattern_coinc_.resize(2);
  pattern_coinc_.set_gates(std::vector<bool>({true, true}));

  pattern_anti_.resize(2);
  pattern_anti_.set_gates(std::vector<bool>({false, false}));

  pattern_add_.resize(2);
  pattern_add_.set_gates(std::vector<bool>({true, true}));

  total_coincidences_ = total_count_;

  std::string name = boost::filesystem::path(filename).filename().string();
  std::replace( name.begin(), name.end(), '.', '_');

//  metadata_.set_attribute(Setting::boolean("symmetric", data->is_symmetric()));

  SpectrumEventMode::_init_from_file(name);
}

void Spectrum2DEvents::_set_detectors(const std::vector<Detector>& dets)
{
  metadata_.detectors.resize(metadata_.dimensions(), Detector());

  if (dets.size() == metadata_.dimensions())
    metadata_.detectors = dets;
  else if (dets.size() > metadata_.dimensions())
  {
    int j=0;
    for (size_t i=0; i < dets.size(); ++i)
    {
      if (metadata_.chan_relevant(i))
      {
        metadata_.detectors[j] = dets[i];
        j++;
        if (j >= metadata_.dimensions())
          break;
      }
    }
  }

  this->_recalc_axes();
}

void Spectrum2DEvents::_recalc_axes()
{
  data_->set_axis(0, DataAxis(Calibration(), pow(2,bits_), bits_));
  data_->set_axis(1, DataAxis(Calibration(), pow(2,bits_), bits_));

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

bool Spectrum2DEvents::event_relevant(const Event& e) const
{
  const auto& c = e.channel();
  return SpectrumEventMode::event_relevant(e) &&
      (e.value(value_idx_[c]).val(bits_) >= cutoff_logic_[c]);
}

void Spectrum2DEvents::add_coincidence(const Coincidence& c)
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
      data_->add({{ll0, ll1}, 1});
}
