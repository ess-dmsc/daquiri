#include "gtest_color_print.h"
#include <core/util/compare.h>

class Compare : public TestBase
{
};

TEST_F(Compare, mid3equal)
{
  EXPECT_EQ(mid(1.0, 1.0, 1.0), 1.0);
}

TEST_F(Compare, mid3ordered)
{
  EXPECT_EQ(mid(1.0, 2.0, 3.0), 2.0);
  EXPECT_EQ(mid(1.0, 2.0, 2.0), 2.0);
  EXPECT_EQ(mid(2.0, 2.0, 3.0), 2.0);
}

TEST_F(Compare, mid3reversed)
{
  EXPECT_EQ(mid(3.0, 2.0, 1.0), 2.0);
  EXPECT_EQ(mid(3.0, 2.0, 2.0), 2.0);
  EXPECT_EQ(mid(2.0, 2.0, 1.0), 2.0);
}

TEST_F(Compare, mid3unordered)
{
  EXPECT_EQ(mid(2.0, 3.0, 1.0), 2.0);
  EXPECT_EQ(mid(2.0, 3.0, 2.0), 2.0);
}

TEST_F(Compare, mid3unordered_b)
{
  EXPECT_EQ(mid(2.0, 1.0, 3.0), 2.0);
  EXPECT_EQ(mid(2.0, 1.0, 2.0), 2.0);
}

TEST_F(Compare, mid3unordered_c)
{
  EXPECT_EQ(mid(3.0, 1.0, 2.0), 2.0);
}

TEST_F(Compare, mid3unordered_d)
{
  EXPECT_EQ(mid(1.0, 3.0, 2.0), 2.0);
}

TEST_F(Compare, mid3same2a)
{
  EXPECT_EQ(mid(4.0, 3.0, 4.0), 4.0);
  EXPECT_EQ(mid(4.0, 5.0, 4.0), 4.0);
}

TEST_F(Compare, mid3same2b)
{
  EXPECT_EQ(mid(3.0, 4.0, 4.0), 4.0);
  EXPECT_EQ(mid(5.0, 4.0, 4.0), 4.0);
}

TEST_F(Compare, mid3same2c)
{
  EXPECT_EQ(mid(4.0, 4.0, 3.0), 4.0);
  EXPECT_EQ(mid(4.0, 4.0, 5.0), 4.0);
}

TEST_F(Compare, mid3same3)
{
  EXPECT_EQ(mid(4.0, 4.0, 4.0), 4.0);
}
