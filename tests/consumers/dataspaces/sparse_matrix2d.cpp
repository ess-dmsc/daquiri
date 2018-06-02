#include <gtest/gtest.h>
#include "sparse_matrix2d.h"

using namespace DAQuiri;

TEST(SparseMatrix2D, Init)
{
  SparseMatrix2D d;
  EXPECT_TRUE(d.empty());
}

