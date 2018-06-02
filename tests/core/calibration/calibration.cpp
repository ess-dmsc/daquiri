#include <gtest/gtest.h>
#include "calibration.h"

using namespace DAQuiri;

TEST(CalibID, Init)
{
  CalibID cid;
  EXPECT_FALSE(cid.valid());
}

TEST(Calibration, Init)
{
  Calibration c;
  EXPECT_FALSE(c.valid());
}
