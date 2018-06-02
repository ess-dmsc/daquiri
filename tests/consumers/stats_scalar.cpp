#include <gtest/gtest.h>
#include "stats_scalar.h"

using namespace DAQuiri;

TEST(StatsScalar, Init)
{
  StatsScalar h;
  EXPECT_EQ(h.type(), "Stats Scalar");
}

