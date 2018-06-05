#pragma once

#include "spectrum.h"
#include "filter_block.h"
#include "value_latch.h"

namespace DAQuiri {

class TimeSpectrum : public Spectrum
{
  public:
    TimeSpectrum();
    TimeSpectrum* clone() const override { return new TimeSpectrum(*this); }

  protected:
    std::string my_type() const override { return "TimeSpectrum 2D"; }

    void _apply_attributes() override;
    void _init_from_file() override;
    void _recalc_axes() override;

    //event processing
    void _push_event(const Event&) override;
    void _push_stats_pre(const Spill& spill) override;

    bool _accept_spill(const Spill& spill) override;
    bool _accept_events(const Spill& spill) override;

    // cached parameters:
    double time_resolution_{1};
    std::string units_name_;
    double units_multiplier_{1};
    FilterBlock filters_;
    ValueLatch value_latch_;

    //from status manifest
    TimeBase timebase_;

    std::vector<double> domain_;

    //reserve memory
    Coords coords_{0, 0};
};

}
