#pragma once

#include "spectrum.h"

class Histogram2D : public Spectrum
{
public:
  Histogram2D();
  Histogram2D* clone() const override
  { return new Histogram2D(*this); }

protected:
  std::string my_type() const override {return "Histogram 2D";}

  bool _initialize() override;
  void _init_from_file() override;
  void _set_detectors(const std::vector<Detector>& dets) override;
  void _recalc_axes() override;

  void _push_event(const Event&) override;
  void _push_stats_pre(const Spill& spill) override;
  void _flush() override;

  bool _accept_spill(const Spill& spill) override;
  bool _accept_events(const Spill& spill) override;

  //cached parameters
  std::string x_name_;
  std::string y_name_;
  uint16_t downsample_ {0};

  //from status manifest
  int x_idx_ {-1};
  int y_idx_ {-1};

  //reserve memory
  Coords coords_ {0, 0};
};
