#pragma once

#include <consumers/add_ons/value_latch.h>
#include <consumers/spectrum.h>

namespace DAQuiri {

class Histogram1D : public Spectrum {
public:
  Histogram1D();
  Histogram1D *clone() const override { return new Histogram1D(*this); }

protected:
  std::string my_type() const override { return "Histogram 1D"; }

  void _apply_attributes() override;
  void _recalc_axes() override;

  // event processing
  void _push_event(const Event &event) override;
  void _push_stats_pre(const Spill &spill) override;
  bool _accept_spill(const Spill &spill) override;
  bool _accept_events(const Spill &spill) override;

  // cached parameters:
  ValueLatch value_latch_;

  // reserve memory
  Coords coords_{0};
};

} // namespace DAQuiri