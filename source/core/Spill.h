/* Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file Spill.h
///
/// \brief key primitive (name?) - defines classes: Spill, EventBuffer
///
/// \todo split in multiple files?
///
//===----------------------------------------------------------------------===//
#pragma once

#include <core/plugin/Setting.h>
#include <core/Detector.h>
#include <core/Event.h>

namespace DAQuiri
{

class Spill;

using SpillPtr = std::shared_ptr<Spill>;
using ListData = std::vector<SpillPtr>;

struct StreamInfo
{
  EventModel event_model;
  Setting stats{Setting::stem("stats")};
};

using StreamManifest = std::map<std::string, StreamInfo>;

class EventBuffer
{
 public:
  EventBuffer() {}

  inline size_t size() const { return data_.size(); }
  inline bool empty() const { return data_.empty(); }

  inline void reserve(size_t s, const Event& e) { data_.resize(s, e); }
  inline Event& last() { return data_[idx_]; }

  inline EventBuffer& operator++()
  {
    idx_++;
    return *this;
  }
  inline EventBuffer operator++(int)
  {
    idx_++;
    return *this;
  }

  inline void finalize() { data_.resize(idx_); }

  inline std::vector<Event>::const_iterator begin() const { return data_.begin(); }
  inline std::vector<Event>::const_iterator end() const { return data_.end(); }

 private:
  std::vector<Event> data_;
  size_t idx_{0};
};

/// \brief contains EventBuffer
/// \todo it is unclear what Spill signifies
class Spill
{
 public:
  enum class Type { start, running, stop, daq_status };

  Spill() {}
  Spill(std::string id, Spill::Type t);

  static Type from_str(const std::string& type);
  static std::string to_str(const Type& type);

  std::string stream_id;
  Type type{Type::daq_status};
  hr_time_t time{std::chrono::system_clock::now()};
  Setting state;

  std::vector<char> raw; // raw from device
  EventModel event_model;
  EventBuffer events;

 public:
  bool empty();
  std::string debug(std::string prepend = "") const;
};

void to_json(json& j, const Spill& s);
void from_json(const json& j, Spill& s);

}
