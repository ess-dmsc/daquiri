#pragma once

#include "coincidence_consumer.h"

class Coincidence1D : public CoincidenceConsumer
{
public:
  Coincidence1D();
  Coincidence1D* clone() const
  { return new Coincidence1D(*this); }

protected:
  std::string my_type() const override {return "1DEvent";}

  bool _initialize() override;
  void _init_from_file(std::string name) override;
  void _set_detectors(const std::vector<Detector>& dets) override;
  void _recalc_axes() override;

  //event processing
  bool event_relevant(const Event&) const override;
  void add_coincidence(const Coincidence&) override;
  virtual void bin_event(const Event&);

  // cached parameters:
  uint16_t bits_ {0};
  uint32_t cutoff_bin_ {0};
};
