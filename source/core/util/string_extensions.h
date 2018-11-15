#pragma once

#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <locale>

// trim from start (in place)
static inline void ltrim(std::string& s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch)
  {
    return !std::isspace(ch);
  }));
}

// trim from end (in place)
static inline void rtrim(std::string& s)
{
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch)
  {
    return !std::isspace(ch);
  }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s)
{
  ltrim(s);
  rtrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s)
{
  ltrim(s);
  return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s)
{
  rtrim(s);
  return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s)
{
  trim(s);
  return s;
}

inline std::string trim_all(std::string text)
{
  std::istringstream iss(text);
  text = "";
  std::string s;
  while (iss >> s)
  {
    if (text != "")
      text += " " + s;
    else text = s;
  }
  return text;
}

template<typename T>
inline std::string join(T strings, std::string delimiter)
{
  std::string ret;
  for (const auto& s : strings)
  {
    if (!ret.empty())
      ret += delimiter;
    ret += s;
  }
  return ret;
}