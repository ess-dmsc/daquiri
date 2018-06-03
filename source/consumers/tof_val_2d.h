#pragma once

#include "spectrum.h"
#include "value_filter.h"

namespace DAQuiri {

class TOFVal2D : public Spectrum
{
  public:
    TOFVal2D();
    TOFVal2D* clone() const override { return new TOFVal2D(*this); }

  protected:
    std::string my_type() const override { return "Time of Flight 2D"; }

    void _apply_attributes() override;
    void _init_from_file() override;
    void _recalc_axes() override;

    //event processing
    void _push_stats_pre(const Spill& spill) override;
    void _push_event(const Event&) override;

    bool _accept_spill(const Spill& spill) override;
    bool _accept_events(const Spill& spill) override;

    // cached parameters:
    double time_resolution_{1};
    std::string units_name_;
    double units_multiplier_{1};
    std::string val_name_;
    uint16_t downsample_{0};
    FilterBlock filters_;

    //from status manifest
    int value_idx_{-1};
    TimeBase timebase_;

    // recent pulse times
    double pulse_time_{-1};

    std::vector<double> domain_;

    //reserve memory
    Coords coords_{0, 0};
};

}
