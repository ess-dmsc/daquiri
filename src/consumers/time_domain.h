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
  void _push_stats(const Status&) override;
  bool channel_relevant(int16_t channel) const override;

  //cached parameters
  Pattern channels_;
  int codomain {0};

  std::vector<PreciseFloat> spectrum_;
  std::vector<PreciseFloat> counts_;
  std::vector<double> seconds_;
  std::vector<Status>  updates_;

  //reserve memory
  Entry entry_ {{0}, 0};
};
