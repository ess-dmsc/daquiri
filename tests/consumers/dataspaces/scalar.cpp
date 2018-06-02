#include <gtest/gtest.h>
#include "scalar.h"

using namespace DAQuiri;

TEST(Scalar, Init)
{
  Scalar d;
  EXPECT_TRUE(d.empty());
}

