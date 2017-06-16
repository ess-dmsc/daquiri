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

#include "hit_model.h"
#include <sstream>

namespace DAQuiri {

void HitModel::add_value(const std::string& name, uint16_t bits)
{
  values.push_back(DigitizedVal(0,bits));
  idx_to_name.push_back(name);
  name_to_idx[name] = values.size() - 1;
}

std::string HitModel::to_string() const
{
  std::stringstream ss;
  ss << "[timebase=" << timebase.to_string() << " ";
  for (auto &n : name_to_idx)
    ss << n.first << "(" << int(values.at(n.second).bits()) << "b) ";
  if (tracelength)
    ss << "trace_length=" << tracelength;
  ss << "]";
  return ss.str();
}

void to_json(json& j, const HitModel& t)
{
  j["timebase"] = t.timebase;
  j["tracelength"] = t.tracelength;
  for (size_t i=0; i < t.values.size(); ++i)
  {
    json jj;
    jj["name"] = t.idx_to_name[i];
    jj["bits"] = t.values[i].bits();
    j["values"].push_back(jj);
  }
}

void from_json(const json& j, HitModel& t)
{
  t.timebase = j["timebase"];
  t.tracelength = j["tracelength"];
  if (j.count("values"))
    for (auto it : j["values"])
      t.add_value(it["name"], it["bits"].get<uint16_t>());
}



}
