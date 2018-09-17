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

TEST_F(TimerTest, restart)
{
  Timer t;
  t.restart();
  EXPECT_LT(t.s(), 1);
  Timer::wait_s(1);
  EXPECT_GT(t.s(), 1.0);

  t.restart();
  EXPECT_LT(t.s(), 1);
  Timer::wait_s(1);
  EXPECT_GT(t.s(), 1.0);
}

TEST_F(TimerTest, construct_start)
{
  Timer t(true);

  EXPECT_LT(t.s(), 1);
  Timer::wait_s(1);
  EXPECT_GT(t.s(), 1.0);
}

TEST_F(TimerTest, construct_with_timeout)
{
  Timer t(1.0, true);

  EXPECT_FALSE(t.timeout());
  Timer::wait_s(1);
  EXPECT_TRUE(t.timeout());
}
