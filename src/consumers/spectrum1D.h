#pragma once

#include "spectrum.h"

class Spectrum1D : virtual public Spectrum
{
public:
  Spectrum1D();

protected:
  bool _initialize() override;
  void _init_from_file(std::string name) override;
  void _recalc_axes() override;

  PreciseFloat _data(std::initializer_list<size_t> list) const override;
  std::unique_ptr<EntryList> _data_range(std::initializer_list<Pair> list) override;
  void _append(const Entry&) override;

  void _set_detectors(const std::vector<Detector>& dets) override;

  void _save_data(H5CC::Group&) const override;
  void _load_data(H5CC::Group&) override;
  std::string _data_debug(const std::string& prepend) const override;

  // cached parameters
  uint16_t bits_ {0};

  // data
  std::vector<PreciseFloat> spectrum_;
  uint16_t maxchan_ {0};
};
