#pragma once

#include <consumers/Spectrum.h>
#include <consumers/add_ons/ValueLatch.h>

namespace DAQuiri {

class Histogram3D : public Spectrum
{
  public:
    Histogram3D();
    Histogram3D* clone() const override { return new Histogram3D(*this); }

  protected:
    std::string my_type() const override { return "Histogram 3D"; }

    void _apply_attributes() override;
    void _recalc_axes() override;

    void _push_event(const Event&) override;
    void _push_stats_pre(const Spill& spill) override;

    bool _accept_spill(const Spill& spill) override;
    bool _accept_events(const Spill& spill) override;

    //cached parameters
    ValueLatch value_latch_x_;
    ValueLatch value_latch_y_;
    ValueLatch value_latch_z_;

    //reserve memory
    Coords coords_{0, 0, 0};
};

}
