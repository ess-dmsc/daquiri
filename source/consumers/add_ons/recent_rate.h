#pragma once

#include <consumers/add_ons/status.h>

namespace DAQuiri {

struct RecentRate
{
  RecentRate() = default;
  RecentRate(const std::string& clock);

  void settings(const Setting& s);
  Setting settings(int32_t index) const;

  Setting update(const Status& current, PreciseFloat new_count);

  // Parameters
  std::string divisor_clock;

  // State
  PreciseFloat previous_count {0};
  Status previous_status;
  PreciseFloat current_rate {0};
};

}