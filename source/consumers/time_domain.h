#pragma once

#include "spectrum.h"

class TimeDomain : public Spectrum
{
public:
  TimeDomain();
  TimeDomain* clone() const override
  { return new TimeDomain(*this); }
  
protected:
  std::string my_type() const override {return "Time-Activity 1D";}

  bool _initialize() override;
  void _init_from_file() override;
  void _set_detectors(const std::vector<Detector>& dets) override;
  void _recalc_axes() override;

  //event processing
  void _push_event(const Event&) override;
  void _push_stats_pre(const Status&) override;
  bool channel_relevant(int16_t channel) const override;

  //cached parameters
  Pattern channels_;
  double time_resolution_ {1};
  std::string units_name_;
  double units_multiplier_{1};

  std::vector<double> domain_;

  //from status manifest
  std::vector<TimeBase> timebase_;

  //reserve memory
  Coords coords_ {0};
};
