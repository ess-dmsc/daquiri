#pragma once

#include <string>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

inline std::string to_max_precision(double number)
{
  std::stringstream ss;
  ss << std::setprecision(std::numeric_limits<double>::max_digits10) << number;
  return ss.str();
}

inline std::string to_str_precision(double number, int precision = -1)
{
  std::ostringstream ss;
  if (precision < 0)
    ss << number;
  else
    ss << std::setprecision(precision) << number;
  return ss.str();
}

inline std::string to_str_decimals(double number, int decimals = 0)
{
  std::stringstream ss;
  ss << std::fixed << std::setprecision(decimals) << number;
  return ss.str();
}

template<typename T> inline bool is_number(T x)
{
  std::string s;
  std::stringstream ss;
  ss << x;
  ss >>s;
  if(s.empty() || std::isspace(s[0]) || std::isalpha(s[0])) return false ;
  char * p ;
  strtod(s.c_str(), &p) ;
  return (*p == 0) ;
}

inline uint16_t sig_digits(std::string st)
{
  boost::to_lower(st);
  boost::replace_all(st, "+", "");
  boost::replace_all(st, "-", "");
  if (boost::contains(st, "e")) {
    size_t l = st.find('e');
    st = st.substr(0,l);
  }
  //assume only one number in string
  uint16_t count=0; bool past_zeros = false;
  for(size_t i=0;i<st.size();i++) {
    bool digit = std::isdigit(st[i]);
    if (digit && (st[i] != '0'))
      past_zeros = true;
    if(past_zeros && digit)
      count++;
  }
  return count;
}

inline int16_t order_of(double val)
{
  if (!std::isfinite(val))
    return 0;
  else
    return std::floor(std::log10(std::abs(val)));
}

inline double get_precision(std::string value)
{
  boost::trim(value);
  boost::trim_if(value, boost::is_any_of("+-"));
  std::vector<std::string> parts;
  boost::split(parts, value, boost::is_any_of("Ee"));

  std::string mantissa;
  std::string expstr;

  if (parts.size() >= 1)
    mantissa = parts.at(0);
  if (parts.size() >= 2)
    expstr = parts.at(1);

  int exponent = 0;
  if (!expstr.empty() && is_number(expstr))
    exponent = boost::lexical_cast<double>(expstr);

  int sigpos = 0;
  size_t pointpos = mantissa.find('.');
  if (pointpos != std::string::npos)
    sigpos -= mantissa.size() - pointpos - 1;

  // return factor for shifting according to exponent
  return pow(10.0, double(sigpos + exponent));
}
