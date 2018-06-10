#pragma once

#include <core/consumer.h>
#include <consumers/add_ons/periodic_trigger.h>
#include <consumers/add_ons/recent_rate.h>
#include <consumers/add_ons/filter_block.h>

namespace DAQuiri {

class Spectrum : public Consumer
{
  public:
    Spectrum();

  protected:
    void _apply_attributes() override;
    bool _accept_spill(const Spill& spill) override;
    void _push_stats_pre(const Spill& spill) override;
    void _push_stats_post(const Spill& spill) override;
    void _flush() override;

  protected:
    PeriodicTrigger periodic_trigger_;
    FilterBlock filters_;

    //TODO: make this parametrizable
    RecentRate recent_rate_{"native_time"};

    std::vector<Status> stats_;

    void update_cumulative(const Status&);
};

}