#include <gtest/gtest.h>
#include "tof_1d.h"

using namespace DAQuiri;

TEST(TOF1D, Init)
{
  TOF1D h;
  EXPECT_EQ(h.type(), "Time of Flight 1D");
}

