#pragma once
#include <gtest/gtest.h>
#include <fmt/color.h>

namespace testing::internal {

extern void ColoredPrintf(GTestColor color, const char* format, ...);
}

class TestBase : public ::testing::Test
{
  protected:
    class Message : public std::stringstream
    {
      public:
        ~Message()
        {
          fmt::print(fg(fmt::terminal_color::green), "[          ] ");
          fmt::print(fg(fmt::terminal_color::yellow), "{}", str());
        }
    };
#define MESSAGE Message
};
