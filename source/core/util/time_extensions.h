#pragma once

#include <string>
#include <sstream>
#include <boost/date_time/local_time/local_time.hpp>

inline boost::posix_time::ptime from_iso_extended(std::string str)
{
  boost::posix_time::ptime tm;
  if (str.empty())
    return tm;
  boost::posix_time::time_input_facet *tif = new boost::posix_time::time_input_facet;
  tif->set_iso_extended_format();
  std::stringstream iss(str);
  iss.imbue(std::locale(std::locale::classic(), tif));
  iss >> tm;
  return tm;
}

inline boost::posix_time::ptime from_custom_format(std::string str, std::string format)
{
  boost::posix_time::ptime tm;
  if (str.empty())
    return tm;
  boost::posix_time::time_input_facet
      *tif(new boost::posix_time::time_input_facet(format));
  std::stringstream iss(str);
  iss.imbue(std::locale(std::locale::classic(), tif));
  iss >> tm;
  return tm;
}

inline std::string very_simple(const boost::posix_time::time_duration &duration)
{
  uint64_t s = duration.total_seconds();
  uint64_t min = s / 60;
  uint64_t h   = min / 60;
  std::stringstream ss;
  ss << h << ":"
     << std::setfill('0') << std::setw(2) << min - (h*60) << ":"
     << std::setfill('0') << std::setw(2) << s - (min*60);
  return ss.str();
}
