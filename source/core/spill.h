#pragma once

#include <core/plugin/setting.h>
#include <core/detector.h>
#include <core/event.h>

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
  using container_t = std::vector<Event>;
 public:
  using const_iterator = typename container_t::const_iterator;
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

  inline const_iterator begin() const { return data_.begin(); }
  inline const_iterator end() const { return data_.end(); }

 private:
  std::vector<Event> data_;
  size_t idx_{0};
};

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
  hr_time_t time{};
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
