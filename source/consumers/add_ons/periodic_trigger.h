#pragma once

#include "status.h"

namespace DAQuiri {

class PeriodicTrigger
{
  public:
    enum ClockType : int32_t
    {
      ProducerWallClock = 0,
      ConsumerWallClock = 1,
      NativeTime = 2
    };

    void settings(const Setting& s);
    Setting settings(int32_t index = -1, std::string override_name = "") const;

    void update(const Status& current);
    void eval_trigger();

    // Parameters
    bool enabled {false};
    ClockType clock_type {ClockType::ProducerWallClock};
    boost::posix_time::time_duration timeout {boost::posix_time::seconds(0)};

    // State
    bool triggered {false};

    //pivate:
    Status previous_;
    boost::posix_time::time_duration recent_time_ {boost::posix_time::seconds(0)};
};

}