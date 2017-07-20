#pragma once

#include "spectrum2D.h"

class Image2D
    : virtual public Spectrum2D
{
public:
  Image2D();
  Image2D* clone() const
  { return new Image2D(*this); }

protected:
  std::string my_type() const override {return "Image2D";}

  bool _initialize() override;
  void _init_from_file(std::string name) override;

  void _push_event(const Event&) override;
  void _push_stats(const Status&) override;
  void _flush() override;

  bool channel_relevant(int16_t channel) const override;
  bool event_relevant(const Event& e) const;

  //cached parameters
  std::string x_name_;
  std::string y_name_;
  Pattern pattern_add_;

  //from status manifest
  std::vector<int> x_idx_;
  std::vector<int> y_idx_;
};
