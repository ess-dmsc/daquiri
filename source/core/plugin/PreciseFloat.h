#pragma once

#include <limits>
#include <string>
#include <sstream>
#include <iomanip>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

//#define PF_DOUBLE 1
#define PF_LONG_DOUBLE 1
//#define PF_DEC_FLOAT 1
//#define PF_MP128 1

#ifdef PF_DOUBLE
using PreciseFloat = double;

inline double to_double(PreciseFloat pf)
{
  return pf;
}

inline PreciseFloat from_double(double d)
{
  return d;
}

inline PreciseFloat from_string(const std::string str)
{
  PreciseFloat ret { std::numeric_limits<double>::quiet_NaN() };
  try { ret = std::stod(str); }
  catch(...) {}
  return ret;
}

inline std::string to_string(const PreciseFloat pf)
{
  std::stringstream ss;
  ss << std::setprecision(std::numeric_limits<PreciseFloat>::max_digits10) << pf;
  return ss.str();
}
#endif


#ifdef PF_DEC_FLOAT
#define FLOAT_PRECISION 16
#define PF_MP 1
#include <boost/multiprecision/cpp_dec_float.hpp>
using PreciseFloat = boost::multiprecision::number<boost::multiprecision::cpp_dec_float<FLOAT_PRECISION> >;
#endif

#ifdef PF_MP128
#define PF_MP 1
#include <boost/multiprecision/float128.hpp>
using PreciseFloat = boost::multiprecision::float128;
#endif

//#include <boost/multiprecision/mpfr.hpp>
//using PreciseFloat = boost::multiprecision::mpfr_float_50;
//#include <quadmath.h>
//using PreciseFloat = __float128;

#ifdef PF_MP

inline double to_double(PreciseFloat pf)
{
  return pf.convert_to<double>();
}

inline PreciseFloat from_double(double d)
{
  return PreciseFloat{ d };
}

inline PreciseFloat from_string(const std::string str)
{
  PreciseFloat ret { std::numeric_limits<double>::quiet_NaN() };
  try { ret = PreciseFloat(str); }
  catch(...) {}
  return ret;
}

inline std::string to_string(const PreciseFloat pf)
{
  std::stringstream ss;
  ss << std::setprecision(std::numeric_limits<PreciseFloat>::max_digits10) << pf;
  return ss.str();
}

#endif


#ifdef PF_LONG_DOUBLE

using PreciseFloat = long double;

inline double to_double(PreciseFloat pf)
{
  return static_cast<double>(pf);
}

inline PreciseFloat from_double(double d)
{
  return static_cast<PreciseFloat>(d);
}

inline PreciseFloat from_string(const std::string str)
{
  PreciseFloat ret { std::numeric_limits<double>::quiet_NaN() };
  try { ret = std::stold(str); }
  catch(...) {}
  return ret;
}

inline std::string to_string(const PreciseFloat pf)
{
  std::stringstream ss;
  ss << std::setprecision(std::numeric_limits<PreciseFloat>::max_digits10) << pf;
  return ss.str();
}

#endif

inline void to_json(json& j, const PreciseFloat& s)
{
  j = to_string(s);
}

inline void from_json(const json& j, PreciseFloat& s)
{
  s = from_string(j.get<std::string>());
}
