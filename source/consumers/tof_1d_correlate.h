#pragma once

#include "spectrum.h"

class TOF1DCorrelate : public Spectrum
{
public:
  TOF1DCorrelate();
  TOF1DCorrelate* clone() const override
  { return new TOF1DCorrelate(*this); }

protected:
  std::string my_type() const override {return "Time of Flight 1D (w/ stream correlation)";}

  bool _initialize() override;
  void _init_from_file() override;
  void _set_detectors(const std::vector<Detector>& dets) override;
  void _recalc_axes() override;

  //event processing
  void _push_stats_pre(const Spill& spill) override;
  void _push_event(const Event& event) override;

  bool _accept_spill(const Spill& spill) override;
  bool _accept_events() override;

  // cached parameters:
  double time_resolution_ {1};
  std::string units_name_;
  double units_multiplier_{1};

  std::string pulse_stream_id_;

  // from status manifest
  TimeBase timebase_;
  TimeBase pulse_timebase_;

  // recent pulse time
  double pulse_time_ {-1};

  std::vector<double> domain_;

  //reserve memory
  Coords coords_ {0};
};
