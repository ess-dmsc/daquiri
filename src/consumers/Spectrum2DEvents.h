#pragma once

#include "spectrum_event_mode.h"
//#include "spectrum2D.h"

class Spectrum2DEvents : public SpectrumEventMode
{
public:
  Spectrum2DEvents();
  Spectrum2DEvents* clone() const
  { return new Spectrum2DEvents(*this); }

protected:
  std::string my_type() const override {return "2DEvent";}

  bool _initialize() override;
  void _init_from_file(std::string name) override;
  void _set_detectors(const std::vector<Detector>& dets) override;
  void _recalc_axes() override;

  //event processing
  bool event_relevant(const Event&) const override;
  void add_coincidence(const Coincidence&) override;

  //indexes of the two chosen channels
  uint16_t bits_ {0};
  std::vector<int8_t> pattern_;
};
