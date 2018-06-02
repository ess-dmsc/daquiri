#include <gtest/gtest.h>
#include "histogram_1d.h"

using namespace DAQuiri;

TEST(Histogram1D, Init)
{
  Histogram1D h;
  EXPECT_EQ(h.type(), "Histogram 1D");
}

