#pragma once

#include "spectrum_event_mode.h"
#include "spectrum2D.h"

class Spectrum2DEvent
    : virtual public SpectrumEventMode
    , virtual public Spectrum2D
{
public:
  Spectrum2DEvent();
  Spectrum2DEvent* clone() const
  { return new Spectrum2DEvent(*this); }

protected:
  std::string my_type() const override {return "2DEvent";}

  bool _initialize() override;
  void _init_from_file(std::string name) override;
  void _recalc_axes() override;

  //event processing
  bool event_relevant(const Event&) const override;
  void add_coincidence(const Coincidence&) override;

  //indexes of the two chosen channels
  std::vector<int8_t> pattern_;
};
