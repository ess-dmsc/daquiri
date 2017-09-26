#pragma once

#include "spectrum.h"

class TOF1D : public Spectrum
{
public:
  TOF1D();
  TOF1D* clone() const override
  { return new TOF1D(*this); }

protected:
  std::string my_type() const override {return "Time of Flight 1D";}

  bool _initialize() override;
  void _init_from_file() override;
  void _set_detectors(const std::vector<Detector>& dets) override;
  void _recalc_axes() override;

  //event processing
  void _push_spill(const Spill&spill) override;
  void _push_event(const Event&) override;
  bool channel_relevant(int16_t channel) const override;

  // cached parameters:
  Pattern channels_;
  double time_resolution_ {1};
  std::string units_name_;
  double units_multiplier_{1};

  //from status manifest
  std::vector<TimeBase> timebase_;

  // recent pulse times
  std::vector<double> pulse_times_;

  std::vector<double> domain_;

  //reserve memory
  Coords coords_ {0};
};
