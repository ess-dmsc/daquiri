#include <gtest/gtest.h>
#include "dense1d.h"

using namespace DAQuiri;

TEST(Dense1D, Init)
{
  Dense1D d;
  EXPECT_TRUE(d.empty());
}

