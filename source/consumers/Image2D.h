#pragma once

#include <consumers/Spectrum.h>
#include <consumers/add_ons/ValueLatch.h>

namespace DAQuiri {

class Image2D : public Spectrum
{
  public:
    Image2D();
    Image2D* clone() const override { return new Image2D(*this); }

  protected:
    std::string my_type() const override { return "Image 2D"; }

    void _apply_attributes() override;
    void _recalc_axes() override;

    void _push_event(const Event&) override;
    void _push_stats_pre(const Spill& spill) override;

    bool _accept_spill(const Spill& spill) override;
    bool _accept_events(const Spill& spill) override;

    //cached parameters
    ValueLatch value_latch_x_;
    ValueLatch value_latch_y_;
    ValueLatch value_latch_i_;

    //reserve memory
    Entry entry_{{0, 0}, 0};
};

}
