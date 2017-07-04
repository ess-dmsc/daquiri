#pragma once

#include "status.h"
#include "setting.h"
#include "detector.h"
#include "hit.h"

namespace DAQuiri {

struct Spill
{
public:
  boost::posix_time::ptime   time
    {boost::posix_time::microsec_clock::universal_time()};
  std::vector<char>          data; // raw from device
  std::list<Hit>             hits; // parsed
  std::map<int16_t, Status> stats; // per channel
  std::vector<Detector> detectors; // per channel
  Setting                   state;

public:
  bool empty();
  std::string to_string() const;
};

typedef std::shared_ptr<Spill> SpillPtr;
typedef std::vector<SpillPtr> ListData;

void to_json(json& j, const Spill &s);
void from_json(const json& j, Spill &s);

}
