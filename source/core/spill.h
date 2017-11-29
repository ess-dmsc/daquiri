#pragma once

#include "setting.h"
#include "detector.h"
#include "event.h"

enum class StatusType { start, running, stop, daq_status };

inline StatusType type_from_str(std::string type)
{
  if (type == "start")
    return StatusType::start;
  else if (type == "stop")
    return StatusType::stop;
  else if (type == "running")
    return StatusType::running;
  else
    return StatusType::daq_status;
}

inline std::string type_to_str(StatusType type)
{
  if (type == StatusType::start)
    return "start";
  else if (type == StatusType::stop)
    return "stop";
  else if (type == StatusType::running)
    return "running";
  else
    return "daq_status";
}

namespace DAQuiri {

class Spill;

using SpillPtr = std::shared_ptr<Spill>;
using ListData = std::vector<SpillPtr>;

class EventBuffer
{
    using container_t = std::vector<Event>;
  public:
    using const_iterator = typename container_t::const_iterator;
    EventBuffer() {}

    inline size_t size() const { return data_.size(); }
    inline bool empty() const { return data_.empty(); }

    inline void reserve(size_t s, const Event &e) { data_.resize(s, e); }
    inline Event& last() { return data_[idx_]; }

    inline EventBuffer& operator++ () { idx_++; return *this; }
    inline EventBuffer operator++ (int) { idx_++; return *this; }

    inline void finalize() { data_.resize(idx_); }

    inline const_iterator begin() const { return data_.begin(); }
    inline const_iterator end() const { return data_.end(); }

  private:
    std::vector<Event> data_;
    size_t idx_ {0};
};

class Spill
{
  public:
    Spill() {}
    Spill(std::string id, StatusType t);

    std::string                stream_id;
    StatusType                 type {StatusType::daq_status};
    boost::posix_time::ptime   time {boost::posix_time::microsec_clock::universal_time()};
    Setting                    state;

    std::vector<char> raw; // raw from device
    EventModel        event_model;
    EventBuffer       events;

  public:
    bool empty();
    std::string to_string() const;
};


void to_json(json& j, const Spill &s);
void from_json(const json& j, Spill &s);

}
