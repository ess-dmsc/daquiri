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

#include "stats_update.h"
#include <boost/date_time.hpp>
#include "setting.h"
#include "detector.h"

namespace DAQuiri {

struct Spill
{
public:
  boost::posix_time::ptime time
    {boost::posix_time::microsec_clock::universal_time()};
  std::vector<uint32_t>  data;  //as is from device, unparsed
  std::list<Hit>    hits;  //as parsed
  std::map<int16_t, StatsUpdate> stats;

  Setting state;
  std::vector<Detector> detectors;

public:
  bool shallow_equals(const Spill& other) const {return (time == other.time);}
  bool operator==(const Spill other) const;
  bool operator<(const Spill other) const  {return (time < other.time);}
  bool operator!=(const Spill other) const {return !operator ==(other);}

  bool empty();
  std::string to_string() const;
};

typedef std::shared_ptr<Spill> SpillPtr;
typedef std::vector<SpillPtr> ListData;

void to_json(json& j, const Spill &s);
void to_json(json& j, const Spill& s, bool with_settings);
void from_json(const json& j, Spill &s);

}
