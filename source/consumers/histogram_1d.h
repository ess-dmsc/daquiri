#pragma once

#include "spectrum.h"

class Histogram1D : public Spectrum
{
public:
  Histogram1D();
  Histogram1D* clone() const override
  { return new Histogram1D(*this); }

protected:
  std::string my_type() const override {return "Histogram 1D";}

  bool _initialize() override;
  void _init_from_file() override;
  void _set_detectors(const std::vector<Detector>& dets) override;
  void _recalc_axes() override;

  //event processing
  void _push_event(const Event& event) override;
  void _push_stats_pre(const Spill&spill) override;
  bool _accept_spill(const Spill& spill) override;
  bool _accept_events() override;

  // cached parameters:
  uint16_t downsample_ {0};
  uint32_t cutoff_bin_ {0};
  std::string val_name_;

  //from status manifest
  int value_idx_ {-1};

  //reserve memory
  Coords coords_ {0};
};
