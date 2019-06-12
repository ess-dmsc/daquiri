#pragma once
#include <gtest/gtest.h>
#include <core/util/color_bash.h>

namespace testing {
namespace internal {
enum GTestColor { COLOR_DEFAULT, COLOR_RED, COLOR_GREEN, COLOR_YELLOW };

extern void ColoredPrintf(GTestColor color, const char* fmt, ...);
}
}

class TestBase : public ::testing::Test
{
  protected:
    class Message : public std::stringstream
    {
      public:
        ~Message()
        {
          std::cout << col(BashColor::GREEN) <<  "[          ] ";
          std::cout << col(BashColor::YELLOW) <<  str();
        }
    };
#define MESSAGE Message
};
