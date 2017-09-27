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

    fb_parser() : Producer() {}
    virtual ~fb_parser() {}

    void boot() override;
    void die() override;

    virtual SpillPtr start_spill() const = 0;
    virtual SpillPtr stop_spill() const = 0;
    virtual SpillPtr process_payload(void*, TimeBase tb,
                                   uint64_t utime,
                                   PayloadStats& stats) = 0;
    virtual SpillPtr dummy_spill(uint64_t utime,
                                   PayloadStats& stats) = 0;

};
