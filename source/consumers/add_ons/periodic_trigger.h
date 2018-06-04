#pragma once

#include "status.h"

namespace DAQuiri {

struct PeriodicTrigger
{
  enum ClearReferenceTime : int32_t
  {
    ProducerWallClock = 0,
    ConsumerWallClock = 1,
    NativeTime = 2
  };

  void settings(const Setting& s);
  Setting settings(int32_t index) const;

  void update(const Status& current);
  void eval_trigger();

  // Parameters
  bool enabled_{false};
  ClearReferenceTime clear_using_{ClearReferenceTime::ProducerWallClock};
  boost::posix_time::time_duration timeout_{boost::posix_time::not_a_date_time};

  // State
  bool triggered_{false};
  Status previous_;
  boost::posix_time::time_duration recent_time_{boost::posix_time::seconds(0)};
};

}