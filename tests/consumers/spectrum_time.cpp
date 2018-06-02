#include <gtest/gtest.h>
#include "spectrum_time.h"

using namespace DAQuiri;

TEST(TimeSpectrum, Init)
{
  TimeSpectrum h;
  EXPECT_EQ(h.type(), "TimeSpectrum 2D");
}

