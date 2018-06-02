#include <gtest/gtest.h>
#include "sparse_map3d.h"

using namespace DAQuiri;

TEST(SparseMap3D, Init)
{
  SparseMap3D d;
  EXPECT_TRUE(d.empty());
}

