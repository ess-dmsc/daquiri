/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 ******************************************************************************/

#pragma once

#include "hit_model.h"
#include <vector>
#include <fstream>

namespace DAQuiri {

class Hit
{
private:
  int16_t       source_channel_;
  TimeStamp     timestamp_;
  std::vector<DigitizedVal>          values_;
  std::vector<std::vector<uint16_t>> traces_;

public:
  inline Hit()
    : Hit(-1, HitModel())
  {}

  inline Hit(int16_t sourcechan, const HitModel &model)
    : source_channel_(sourcechan)
    , timestamp_(0, model.timebase)
    , values_ (model.values)
  {
    for (auto t : model.traces)
    {
      size_t product = 1;
      for (auto d : t)
        product *= d;
      traces_.push_back(std::vector<uint16_t>(product, 0));
    }
  }

  //Accessors
  inline const int16_t& source_channel() const
  {
    return source_channel_;
  }

  inline const TimeStamp& timestamp() const
  {
    return timestamp_;
  }

  inline size_t value_count() const
  {
    return values_.size();
  }

  inline DigitizedVal value(size_t idx) const
  {
    if (idx >= values_.size())
      throw std::out_of_range("Hit: bad value index");
    return values_.at(idx);
  }

  inline const std::vector<uint16_t>& trace(size_t idx) const
  {
    if (idx >= traces_.size())
      throw std::out_of_range("Hit: bad trace index");
    return traces_.at(idx);
  }

  //Setters
  inline void set_timestamp(TimeStamp ts)
  {
    timestamp_ = ts;
  }

  inline void set_value(size_t idx, uint16_t val)
  {
    if (idx < values_.size())
      values_[idx].set_val(val);
  }

  inline void set_trace(size_t idx, const std::vector<uint16_t> &trc)
  {
    if (idx >= traces_.size())
      throw std::out_of_range("Hit: bad trace index");
    auto& t = traces_[idx];
    size_t len = std::min(trc.size(), t.size());
    for (size_t i=0; i < len; ++i)
      t[i] = trc.at(i);
  }

  //Comparators
  inline bool operator==(const Hit other) const
  {
    if (source_channel_ != other.source_channel_) return false;
    if (timestamp_ != other.timestamp_) return false;
    if (values_ != other.values_) return false;
    if (traces_ != other.traces_) return false;
    return true;
  }

  inline bool operator!=(const Hit other) const
  {
    return !operator==(other);
  }

  inline bool operator<(const Hit other) const
  {
    return (timestamp_ < other.timestamp_);
  }

  inline bool operator>(const Hit other) const
  {
    return (timestamp_ > other.timestamp_);
  }

  inline void delay_ns(double ns)
  {
    timestamp_.delay(ns);
  }

  inline std::string debug() const
  {
    std::stringstream ss;
    ss << "[ch" << source_channel_ << "|t" << timestamp_.debug();
    for (auto &v : values_)
      ss << v.debug();
    ss << "]";
    return ss.str();
  }
};

}
