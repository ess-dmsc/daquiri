#pragma once

#include "status.h"
#include "setting.h"
#include "detector.h"
#include "event.h"

namespace DAQuiri {

class Spill;

using SpillPtr = std::shared_ptr<Spill>;
using ListData = std::vector<SpillPtr>;

class Spill
{
  public:
    boost::posix_time::ptime   time
    {boost::posix_time::microsec_clock::universal_time()};
    std::vector<char>          data;   // raw from device
    std::vector<Event>         events; // parsed
    std::map<int16_t, Status> stats; // per channel
    std::vector<Detector> detectors; // per channel
    Setting                   state;

  public:
    bool empty();
    std::string to_string() const;
    static SpillPtr make_new(StatusType t,
                             std::initializer_list<int16_t> channels);
};


void to_json(json& j, const Spill &s);
void from_json(const json& j, Spill &s);

}
