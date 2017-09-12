#pragma once

#include "spectrum.h"

class Image2D : public Spectrum
{
public:
  Image2D();
  Image2D* clone() const override
  { return new Image2D(*this); }

protected:
  std::string my_type() const override {return "Image 2D";}

  bool _initialize() override;
  void _init_from_file() override;
  void _set_detectors(const std::vector<Detector>& dets) override;
  void _recalc_axes() override;

  void _push_event(const Event&) override;
  void _push_stats(const Status&) override;
  void _flush() override;

  bool channel_relevant(int16_t channel) const override;
  bool event_relevant(const Event& e) const;

  //cached parameters
  std::string x_name_;
  std::string y_name_;
  std::string val_name_;
  Pattern add_channels_;
  uint16_t downsample_ {0};

  //from status manifest
  std::vector<int> x_idx_;
  std::vector<int> y_idx_;
  std::vector<int> val_idx_;
};
