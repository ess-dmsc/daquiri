#pragma once

#include <consumers/add_ons/value_filter.h>

namespace DAQuiri {

struct FilterBlock {
  void settings(const Setting &s);
  Setting settings() const;

  void configure(const Spill &spill);

  inline bool accept(const Event &event) const {
    if (!valid)
      return true;
    for (auto &f : filters_)
      if (!f.accept(event))
        return false;
    return true;
  }

  std::vector<ValueFilter> filters_;
  bool valid{false};
};

} // namespace DAQuiri
