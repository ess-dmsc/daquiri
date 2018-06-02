#include <gtest/gtest.h>
#include "sparse_map2d.h"

using namespace DAQuiri;

TEST(SparseMap2D, Init)
{
  SparseMap2D d;
  EXPECT_TRUE(d.empty());
}

