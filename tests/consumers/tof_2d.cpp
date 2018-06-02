#include <gtest/gtest.h>
#include "tof_val_2d.h"

using namespace DAQuiri;

TEST(TOFVal2D, Init)
{
  TOFVal2D h;
  EXPECT_EQ(h.type(), "Time of Flight 2D");
}

