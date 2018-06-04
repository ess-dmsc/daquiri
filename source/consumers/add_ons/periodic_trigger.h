#pragma once

#include "status.h"

namespace DAQuiri {

enum ClearReferenceTime : int32_t
{
  ProducerWallClock = 0,
  ConsumerWallClock = 1,
  NativeTime = 2
};

struct PeriodicTrigger
{
  bool triggered_{false};

  bool enabled_{false};
  ClearReferenceTime clear_using_{ClearReferenceTime::ProducerWallClock};
  boost::posix_time::time_duration timeout_{boost::posix_time::not_a_date_time};

  void settings(const Setting& s);
  Setting settings(int32_t index) const;

  void update_times(const Status& from, const Status& to);
  void eval_trigger();

  boost::posix_time::time_duration recent_time_{boost::posix_time::seconds(0)};
};

}