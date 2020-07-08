/* Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file Event.h
///
/// \brief key primitive for storing event data
///
//===----------------------------------------------------------------------===//
#pragma once

#include <core/EventModel.h>
#include <vector>

namespace DAQuiri {

class Event
{
private:
  uint64_t      timestamp_ {0};
  std::vector<uint32_t>              values_;
  std::vector<std::vector<uint32_t>> traces_;

public:
  inline Event() {}

  inline Event(const EventModel &model)
    : values_ (model.values)
  {
    for (auto t : model.traces)
    {
      size_t product = 1;
      for (auto d : t)
        product *= d;
      traces_.push_back(std::vector<uint32_t>(product, 0));
    }
  }

  //Accessors
  inline uint64_t timestamp() const
  {
    return timestamp_;
  }

  inline size_t value_count() const
  {
    return values_.size();
  }

  inline size_t trace_count() const
  {
    return traces_.size();
  }

  inline uint32_t value(size_t idx) const
  {
    return values_[idx];
  }

  inline std::vector<uint32_t>& trace(size_t idx)
  {
    if (idx >= traces_.size())
      throw std::out_of_range("Event: bad trace index");
    return traces_[idx];
  }

  inline const std::vector<uint32_t>& trace(size_t idx) const
  {
    if (idx >= traces_.size())
      throw std::out_of_range("Event: bad trace index");
    return traces_[idx];
  }

  //Setters
  inline void set_time(uint64_t t)
  {
    timestamp_ = t;
  }

  inline void set_value(size_t idx, uint32_t val)
  {
    values_[idx] = val;
  }

  //Comparators
  inline bool operator==(const Event other) const
  {
    if (timestamp_ != other.timestamp_) return false;
    if (values_ != other.values_) return false;
    if (traces_ != other.traces_) return false;
    return true;
  }

  inline bool operator!=(const Event other) const
  {
    return !operator==(other);
  }

  inline std::string debug() const
  {
    std::stringstream ss;
    ss << "[t" << timestamp_;
    if (traces_.size())
      ss << "|ntraces=" << traces_.size();
    for (auto &v : values_)
      ss << " " << v;
    ss << "]";
    return ss.str();
  }
};

}
