#include <core/util/time_extensions.h>

#include <sstream>
#include <date/date.h>

/*
boost::posix_time::ptime from_custom_format(std::string str, std::string format)
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
*/

std::string very_simple(const hr_duration_t &duration)
{
  using namespace date;
  std::stringstream ss;
  ss << format("%T", round<std::chrono::seconds>(duration));
  return ss.str();
}

std::string to_simple(const hr_duration_t &duration)
{
  using namespace date;
  std::stringstream ss;
  ss << format("%T", round<std::chrono::nanoseconds>(duration));
  return ss.str();
}

hr_duration_t duration_from_string(const std::string& durstring)
{
  using namespace date;
  std::istringstream in{durstring};
  std::chrono::nanoseconds ns;
  in >> parse("%6H:%M:%S", ns);
  return ns;
}

std::string to_simple(const hr_time_t& time)
{
  using namespace date;
  std::stringstream ss;
  ss << format("%F %T", floor<std::chrono::seconds>(time));
  return ss.str();
}

std::string to_iso_extended(const hr_time_t& time)
{
  using namespace date;
  std::stringstream ss;
  ss << format("%FT%T", floor<std::chrono::nanoseconds>(time));
  return ss.str();
}

hr_time_t from_iso_extended(const std::string& timestr)
{
  using namespace date;
  std::istringstream in{timestr};
  hr_time_t ret;
  in >> parse("%FT%T", ret);
  return ret;
}
