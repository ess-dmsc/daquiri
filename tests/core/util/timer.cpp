#include "gtest_color_print.h"
#include <core/util/timer.h>
#include <date/date.h>

class TimerTest : public TestBase
{
};

TEST_F(TimerTest, default_constructed)
{
  Timer t;
  EXPECT_FALSE(t.timeout());
}
