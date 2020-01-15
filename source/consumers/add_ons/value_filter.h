#pragma once

#include <core/spill.h>

namespace DAQuiri {

struct ValueFilter {
  void settings(const Setting &s);
  Setting settings(int32_t index) const;

  void configure(const Spill &spill);

  inline bool accept(const Event &event) const {
    if (!valid())
      return true;
    value_ = event.value(idx_);
    return ((value_ >= min_) && (value_ <= max_));
  }

  inline bool valid() const { return (enabled_ && (idx_ >= 0)); }

  bool enabled_{false};
  std::string name_;
  uint32_t min_{0};
  uint32_t max_{std::numeric_limits<int32_t>::max()};

  int idx_{-1};

  mutable uint32_t value_;
};

} // namespace DAQuiri
