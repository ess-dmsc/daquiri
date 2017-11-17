#pragma once

#include "spectrum.h"

class Prebinned1D : public Spectrum
{
public:
  Prebinned1D();
  Prebinned1D* clone() const override
  { return new Prebinned1D(*this); }

protected:
  std::string my_type() const override {return "Prebinned 1D";}

  bool _initialize() override;
  void _init_from_file() override;
  void _set_detectors(const std::vector<Detector>& dets) override;
  void _recalc_axes() override;

  //event processing
  void _push_event(const Event&) override;
  void _push_stats_pre(const Status&) override;
  bool channel_relevant(int16_t channel) const override;

  // cached parameters:
  Pattern channels_;
  std::string trace_name_;

  //from status manifest
  std::vector<int> trace_idx_;

  std::vector<double> domain_;

  //reserve memory
  Entry entry_ {{0}, 0};
};
