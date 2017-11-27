#pragma once

//#include "status.h"
#include "setting.h"
#include "detector.h"
#include "event.h"

enum class StatusType { start, running, stop };

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
    Spill(StatusType t);

    std::string                stream_id;
    StatusType                 type {StatusType::running};
    boost::posix_time::ptime   time {boost::posix_time::microsec_clock::universal_time()};
//    std::map<int16_t, Status>  stats; // per channel
//    std::vector<Detector>      detectors; // per channel
    Setting                    state;

    std::vector<char> raw; // raw from device
    EventModel        event_model;
    EventBuffer       events;

  public:
    bool empty();
    std::string to_string() const;
//    static SpillPtr make_new(StatusType t,
//                             std::initializer_list<int16_t> channels);
};


void to_json(json& j, const Spill &s);
void from_json(const json& j, Spill &s);

}
