#pragma once

#include <consumers/add_ons/status.h>

namespace DAQuiri {

class PeriodicTrigger {
public:
  enum ClockType : int32_t {
    ProducerWallClock = 0,
    ConsumerWallClock = 1,
    NativeTime = 2
  };

  void settings(const Setting &s);
  Setting settings(int32_t index = -1, std::string override_name = "") const;

  void update(const Status &current);
  void eval_trigger();

  // Parameters
  bool enabled{false};
  ClockType clock_type{ClockType::ProducerWallClock};
  hr_duration_t timeout{std::chrono::seconds(0)};

  // State
  bool triggered{false};

  // pivate:
  Status previous_;
  hr_duration_t recent_time_{std::chrono::seconds(0)};
};

} // namespace DAQuiri