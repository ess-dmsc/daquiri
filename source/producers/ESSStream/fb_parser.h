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

    PayloadStats stats;

    fb_parser() : Producer() {}
    virtual ~fb_parser() {}

    void boot() override;
    void die() override;

    virtual uint64_t process_payload(SpillQueue spill_queue, void* msg) = 0;
    virtual uint64_t stop(SpillQueue spill_queue) = 0;
};
