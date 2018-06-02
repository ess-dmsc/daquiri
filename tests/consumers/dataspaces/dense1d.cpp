#include <gtest/gtest.h>
#include "dense1d.h"

using namespace DAQuiri;

TEST(Dense1D, Init)
{
  Dense1D d;
  EXPECT_TRUE(d.empty());
  EXPECT_EQ(d.dimensions(), 1);
  EXPECT_EQ(d.total_count(), 0);
}

TEST(Dense1D, AddOne)
{
  Dense1D d;

  d.add_one({0});
  EXPECT_FALSE(d.empty());
  EXPECT_EQ(d.total_count(), 1);

  d.add_one({0});
  EXPECT_EQ(d.total_count(), 2);
}

TEST(Dense1D, Get)
{
  Dense1D d;

  d.add_one({0});
  EXPECT_EQ(d.get({0}), 1);

  d.add_one({0});
  EXPECT_EQ(d.get({0}), 2);
}

TEST(Dense1D, Range)
{
  Dense1D d;

  d.add_one({0});
  EXPECT_EQ(d.range({})->at(0).second, 1);

  d.add_one({1});
  EXPECT_EQ(d.range({})->at(1).second, 1);
}

