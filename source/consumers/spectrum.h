#pragma once

#include "consumer.h"
#include "periodic_trigger.h"
#include "recent_rate.h"

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
    //TODO: make this parametrizable
    RecentRate recent_rate_{"native_time"};

    std::vector<Status> stats_;

    void update_cumulative(const Status&);
};

}