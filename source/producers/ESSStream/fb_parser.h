#pragma once

#include "producer.h"
#include "custom_timer.h"

using namespace DAQuiri;

class EventMessage;

class fb_parser : public Producer
{
  public:
    struct PayloadStats
    {
        uint64_t time_start {0};
        uint64_t time_end {0};
        double time_spent {0};
    };

    TimeBase timebase;
    PayloadStats stats;

    fb_parser() : Producer() {}
    virtual ~fb_parser() {}

    void boot() override;
    void die() override;

    virtual uint64_t start_spill(SpillQueue spill_queue) const = 0;
    virtual uint64_t stop_spill(SpillQueue spill_queue) const = 0;
    virtual uint64_t dummy_spill(SpillQueue spill_queue, uint64_t utime) = 0;
    virtual uint64_t process_payload(SpillQueue spill_queue,
                                     void*, uint64_t utime) = 0;
};
