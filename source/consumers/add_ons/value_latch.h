#pragma once

#include "spill.h"

namespace DAQuiri {

struct ValueLatch
{
  void settings(const Setting& s);
  Setting settings(int32_t index = -1, std::string override_name = "") const;

  inline bool has_declared_value(const Spill& spill)
  {
    return (0 != spill.event_model.name_to_val.count(value_id_));
  }

  void configure(const Spill& spill);

  inline void extract(size_t& bin, const Event& event) const
  {
    if (downsample_)
      bin = event.value(static_cast<size_t>(idx_)) >> downsample_;
    else
      bin = event.value(static_cast<size_t>(idx_));
  }

  inline bool valid() const
  {
    return (idx_ >= 0);
  }

  std::string value_id_;
  uint16_t downsample_ {0};
  int32_t idx_ {-1};
};

}
