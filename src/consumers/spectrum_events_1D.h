#pragma once

#include "spectrum_event_mode.h"
#include "spectrum1D.h"

class Spectrum1DEvent
    : virtual public SpectrumEventMode
    , virtual public Spectrum1D
{
public:
  Spectrum1DEvent();
  Spectrum1DEvent* clone() const
  { return new Spectrum1DEvent(*this); }

protected:
  std::string my_type() const override {return "1DEvent";}

  bool _initialize() override;
  void _init_from_file(std::string name) override;
  void _recalc_axes() override;

  //event processing
  bool event_relevant(const Event&) const override;
  void add_coincidence(const Coincidence&) override;
  virtual void bin_event(const Event&);

  // cached parameters:
  uint32_t cutoff_bin_ {0};
};
