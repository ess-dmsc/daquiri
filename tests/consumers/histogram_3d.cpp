#include <gtest/gtest.h>
#include "histogram_3d.h"

using namespace DAQuiri;

TEST(Histogram3D, Init)
{
  Histogram3D h;
  EXPECT_EQ(h.type(), "Histogram 3D");
}

