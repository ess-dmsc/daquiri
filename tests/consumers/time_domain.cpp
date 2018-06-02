#include <gtest/gtest.h>
#include "time_domain.h"

using namespace DAQuiri;

TEST(TimeDomain, Init)
{
  TimeDomain h;
  EXPECT_EQ(h.type(), "Time-Activity 1D");
}

