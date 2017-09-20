#pragma once

#include "coincidence_consumer.h"

class Coincidence2D : public CoincidenceConsumer
{
public:
  Coincidence2D();
  Coincidence2D* clone() const override
  { return new Coincidence2D(*this); }

protected:
  std::string my_type() const override {return "Coincidence 2D";}

  bool _initialize() override;
  void _init_from_file() override;
  void _set_detectors(const std::vector<Detector>& dets) override;
  void _recalc_axes() override;

  //event processing
  bool event_relevant(const Event&) const override;
  void add_coincidence(const Coincidence&) override;

  //indexes of the two chosen channels
  uint16_t downsample_ {0};
  std::vector<int8_t> pattern_;
};
