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

#include "status.h"
#include "setting.h"
#include "detector.h"

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
