#include <gtest/gtest.h>
#include "histogram_2d.h"

using namespace DAQuiri;

TEST(Histogram2D, Init)
{
  Histogram2D h;
  EXPECT_EQ(h.type(), "Histogram 2D");
}

