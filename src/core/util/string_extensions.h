#pragma once

#include <string>
#include <sstream>

inline std::string trim_all(std::string text)
{
  std::istringstream iss(text);
  text = "";
  std::string s;
  while(iss >> s)
  {
    if ( text != "" ) text += " " + s;
    else text = s;
  }
  return text;
}
