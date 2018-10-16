#include "gtest_color_print.h"

#include <consumers/dataspaces/dense1d.h>

class Dense1D : public TestBase
{
  protected:
    DAQuiri::Dense1D d;
};

TEST_F(Dense1D, Init)
{
  EXPECT_TRUE(d.empty());
  EXPECT_EQ(d.dimensions(), 1);
  EXPECT_EQ(d.total_count(), 0);
}

TEST_F(Dense1D, AddOne)
{
  d.add_one({0});
  EXPECT_FALSE(d.empty());
  EXPECT_EQ(d.total_count(), 1);

  d.add_one({0});
  EXPECT_EQ(d.total_count(), 2);
}

TEST_F(Dense1D, Get)
{
  d.add_one({0});
  EXPECT_EQ(d.get({0}), 1);

  d.add_one({0});
  EXPECT_EQ(d.get({0}), 2);
}

TEST_F(Dense1D, Add)
{
  d.add({{0}, 3});
  EXPECT_EQ(d.get({0}), 3);

  d.add({{0}, 5});
  EXPECT_EQ(d.get({0}), 8);
}

TEST_F(Dense1D, Clear)
{
  d.add({{0}, 3});
  EXPECT_EQ(d.total_count(), 3);

  d.clear();
  EXPECT_EQ(d.total_count(), 0);
  EXPECT_TRUE(d.empty());
}

TEST_F(Dense1D, Range)
{
  d.add_one({0});
  EXPECT_EQ(d.range({})->at(0).second, 1);
  EXPECT_EQ(d.range({})->at(0).first[0], 0UL);

  d.add_one({1});
  EXPECT_EQ(d.range({})->at(1).second, 1);
  EXPECT_EQ(d.range({})->at(1).first[0], 1UL);
}

TEST_F(Dense1D, Clone)
{
  d.add_one({0});
  d.add_one({1});

  auto d2 = std::shared_ptr<DAQuiri::Dataspace>(d.clone());
  EXPECT_EQ(d2->get({0}), 1);
  EXPECT_EQ(d2->get({1}), 1);
  EXPECT_EQ(d2->dimensions(), 1);
  EXPECT_EQ(d2->total_count(), 2);
}

TEST_F(Dense1D, CalcAxes)
{

  d.add_one({0});
  EXPECT_TRUE(d.axis(0).domain.empty());
  d.recalc_axes();
  EXPECT_EQ(d.axis(0).domain.size(), 1UL);

  d.add_one({1});
  EXPECT_EQ(d.axis(0).domain.size(), 1UL);
  d.recalc_axes();
  EXPECT_EQ(d.axis(0).domain.size(), 2UL);
}

TEST_F(Dense1D, SaveLoadEmpty)
{
  auto f = hdf5::file::create("dummy.h5", hdf5::file::AccessFlags::TRUNCATE);
  auto g = f.root().create_group("empty");
  d.save(g);
  d.load(g);
  EXPECT_TRUE(d.empty());
}

TEST_F(Dense1D, SaveLoadNonempty)
{
  d.add({{0}, 3});

  auto f = hdf5::file::create("dummy.h5", hdf5::file::AccessFlags::TRUNCATE);
  auto g = f.root().create_group("nonempty");
  d.save(g);
  d.load(g);
  EXPECT_FALSE(d.empty());
  EXPECT_EQ(d.total_count(), 3);
}

TEST_F(Dense1D, SaveLoadThrow)
{
  hdf5::node::Group g;

  EXPECT_THROW(d.save(g), std::runtime_error);
  EXPECT_THROW(d.load(g), std::runtime_error);
}

TEST_F(Dense1D, ExportCSV)
{
  d.add_one({0});
  d.add_one({2});

  std::stringstream ss;
  d.export_csv(ss);

  EXPECT_EQ(ss.str(), "1, 0, 1");
}

TEST_F(Dense1D, Debug)
{
  d.add_one({0});
  d.add_one({2});

  MESSAGE() << d.debug() << "\n";
}
