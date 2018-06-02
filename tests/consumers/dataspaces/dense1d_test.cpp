#include "dense1d.h"
#include <gtest/gtest.h>

TEST(Dense1D, Init)
{
  DAQuiri::Dense1D d;
  EXPECT_TRUE(d.empty());
}

