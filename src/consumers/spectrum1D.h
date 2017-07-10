#pragma once

#include "spectrum.h"

class Spectrum1D : virtual public Spectrum
{
public:
  Spectrum1D();

protected:
  std::string my_type() const override {return "1D";}
  bool _initialize() override;
  void _init_from_file(std::string name) override;

  PreciseFloat _data(std::initializer_list<size_t> list) const override;
  std::unique_ptr<EntryList> _data_range(std::initializer_list<Pair> list) override;
  void _append(const Entry&) override;
  void _set_detectors(const std::vector<Detector>& dets) override;

  //dummy
  void _load_data(H5CC::Group&) override;
  void _save_data(H5CC::Group&) const override;

  std::vector<PreciseFloat> spectrum_;
  uint16_t maxchan_ {0};
  uint16_t bits_ {0};
};
