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

TEST(Dense1D, Add)
{
  Dense1D d;

  d.add({{0},3});
  EXPECT_EQ(d.get({0}), 3);

  d.add({{0},5});
  EXPECT_EQ(d.get({0}), 8);
}

TEST(Dense1D, Clear)
{
  Dense1D d;
  d.add({{0},3});
  EXPECT_EQ(d.total_count(), 3);

  d.clear();
  EXPECT_EQ(d.total_count(), 0);
  EXPECT_TRUE(d.empty());
}

TEST(Dense1D, Range)
{
  Dense1D d;

  d.add_one({0});
  EXPECT_EQ(d.range({})->at(0).second, 1);
  EXPECT_EQ(d.range({})->at(0).first[0], 0);

  d.add_one({1});
  EXPECT_EQ(d.range({})->at(1).second, 1);
  EXPECT_EQ(d.range({})->at(1).first[0], 1);
}

TEST(Dense1D, Save)
{
  Dense1D d;
  auto f = hdf5::file::create("dummy.h5", hdf5::file::AccessFlags::TRUNCATE);

  auto empty = f.root().create_group("empty");
  d.save(empty);
}
