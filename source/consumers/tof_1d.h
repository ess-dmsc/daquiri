#pragma once

#include "spectrum.h"

namespace DAQuiri {

class TOF1D : public Spectrum
{
  public:
    TOF1D();
    TOF1D* clone() const override { return new TOF1D(*this); }

  protected:
    std::string my_type() const override { return "Time of Flight 1D"; }

    void _apply_attributes() override;
    void _init_from_file() override;
    void _recalc_axes() override;

    //event processing
    void _push_stats_pre(const Spill& spill) override;
    void _push_event(const Event& event) override;

    bool _accept_spill(const Spill& spill) override;
    bool _accept_events(const Spill& spill) override;

    // cached parameters:
    double time_resolution_{1};
    std::string units_name_;
    double units_multiplier_{1};

    // from status manifest
    TimeBase timebase_;

    // recent pulse time
    double pulse_time_{-1};

    std::vector<double> domain_;

    //reserve memory
    Coords coords_{0};
};

}
