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
  void _push_stats_pre(const Status&) override;
  void _flush() override;

  bool channel_relevant(int16_t channel) const override;
  bool event_relevant(const Event& e) const;

  //cached parameters
  std::string x_name_;
  std::string y_name_;
  Pattern add_channels_;
  uint16_t downsample_ {0};

  //from status manifest
  std::vector<int> x_idx_;
  std::vector<int> y_idx_;

  //reserve memory
  Coords coords_ {0, 0};
};
