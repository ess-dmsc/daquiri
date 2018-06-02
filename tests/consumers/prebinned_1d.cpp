#include <gtest/gtest.h>
#include "prebinned_1d.h"

using namespace DAQuiri;

TEST(Prebinned1D, Init)
{
  Prebinned1D h;
  EXPECT_EQ(h.type(), "Prebinned 1D");
}

