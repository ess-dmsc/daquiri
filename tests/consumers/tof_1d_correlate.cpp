#include <gtest/gtest.h>
#include "tof_1d_correlate.h"

using namespace DAQuiri;

TEST(TOF1DCorrelate, Init)
{
  TOF1DCorrelate h;
  EXPECT_EQ(h.type(), "Time of Flight 1D (w/ stream correlation)");
}

