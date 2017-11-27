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
  void _push_stats_pre(const Spill&spill) override;
  void _flush() override;

  bool _accept_spill(const Spill& spill) override;
  bool _accept_events() override;

  //cached parameters
  std::string x_name_;
  std::string y_name_;
  std::string val_name_;
  uint16_t downsample_ {0};

  //from status manifest
  int x_idx_ {-1};
  int y_idx_ {-1};
  int val_idx_ {-1};

  //reserve memory
  Entry entry_ {{0, 0}, 0};
};
