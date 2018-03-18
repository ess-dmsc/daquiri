#pragma once

#include "spill.h"

struct ValueFilter
{
  void settings(const DAQuiri::Setting& s);
  DAQuiri::Setting settings(int32_t index) const;

  void configure(const DAQuiri::Spill& spill)
  {
    if (spill.event_model.name_to_val.count(name_))
      idx_ = spill.event_model.name_to_val.at(name_);
    else
      idx_ = -1;
  }

  bool accept(const DAQuiri::Event& event) const
  {
    if (idx_ < 0)
      return false;
    value_ = event.value(idx_);
    return ((value_ >= min_) && (value_ <= max_));
  }

  bool enabled_ {false};
  std::string name_;
  uint32_t min_ {0};
  uint32_t max_ {std::numeric_limits<uint32_t>::max()};

  int idx_ {-1};

  mutable uint32_t value_;
};

struct FilterBlock
{
  void settings(const DAQuiri::Setting& s);
  DAQuiri::Setting settings() const;

  void configure(const DAQuiri::Spill& spill)
  {
    for (auto& f : filters_)
      if (f.enabled_)
        f.configure(spill);
  }

  bool accept(const DAQuiri::Event& event) const
  {
    for (auto& f : filters_)
      if (f.enabled_ && !f.accept(event))
        return false;
    return  true;
  }

  std::vector<ValueFilter> filters_;
};