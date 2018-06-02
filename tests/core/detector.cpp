#include <gtest/gtest.h>
#include "detector.h"

using namespace DAQuiri;

TEST(Detector, Init)
{
  Detector d;
  EXPECT_EQ(d.type(), "unknown");
}
