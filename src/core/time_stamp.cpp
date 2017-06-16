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

#include "time_stamp.h"
//#include "util.h"
#include <sstream>

namespace DAQuiri {

std::string TimeStamp::to_string() const
{
  std::stringstream ss;
  ss << time_native_ << "x(" << timebase_multiplier_ << "/" << timebase_divider_ << ")";
  return ss.str();
}

void to_json(json& j, const TimeStamp& t)
{
  j["multiplier"] = t.timebase_multiplier();
  j["divider"] = t.timebase_divider();
}

void from_json(const json& j, TimeStamp& t)
{
  t = TimeStamp(j["multiplier"], j["divider"]);
}


}
