#include "gtest_color_print.h"

#include <consumers/dataspaces/scalar.h>

class Scalar : public TestBase
{
  protected:
    DAQuiri::Scalar d;
};

TEST_F(Scalar, Init)
{
  EXPECT_TRUE(d.empty());
  EXPECT_EQ(d.dimensions(), 0);
  EXPECT_EQ(d.total_count(), 0);
}

TEST_F(Scalar, AddOne)
{
  d.add_one({});
  EXPECT_FALSE(d.empty());
  EXPECT_EQ(d.total_count(), 1);

  d.add_one({});
  EXPECT_EQ(d.total_count(), 2);
}

TEST_F(Scalar, Get)
{
  d.add_one({});
  EXPECT_EQ(d.get({}), 1);

  d.add_one({});
  EXPECT_EQ(d.get({}), 2);
}

TEST_F(Scalar, Overwirte)
{
  d.add({{}, 3});
  EXPECT_EQ(d.get({}), 3);

  d.add({{}, 5});
  EXPECT_EQ(d.get({}), 5);
}

TEST_F(Scalar, Clear)
{
  d.add_one({});
  EXPECT_EQ(d.total_count(), 1);

  d.clear();
  EXPECT_EQ(d.total_count(), 0);
  EXPECT_TRUE(d.empty());
}

TEST_F(Scalar, GetMinMax)
{
  //using range to get min max

  d.add({{}, 3});
  d.add({{}, 42});
  d.add({{}, 9});

  EXPECT_EQ(d.get({}), 9);
  EXPECT_EQ(d.range({})->begin()->second, 3);
  EXPECT_EQ(d.range({})->rbegin()->second, 42);
}

TEST_F(Scalar, Clone)
{
  d.add({{}, 3});
  d.add({{}, 42});
  d.add({{}, 9});

  auto d2 = std::shared_ptr<DAQuiri::Dataspace>(d.clone());
  EXPECT_EQ(d2->get({}), 9);
  EXPECT_EQ(d2->range({})->begin()->second, 3);
  EXPECT_EQ(d2->range({})->rbegin()->second, 42);
  EXPECT_EQ(d2->dimensions(), 0);
  EXPECT_EQ(d2->total_count(), 3);
}

TEST_F(Scalar, CalcAxes)
{
  //TODO: this feels wrong

  d.add_one({});
  EXPECT_TRUE(d.axis(0).domain.empty());
  d.recalc_axes();
  EXPECT_EQ(d.axis(0).domain.size(), 0UL);

  d.add_one({});
  EXPECT_EQ(d.axis(0).domain.size(), 0UL);
  d.recalc_axes();
  EXPECT_EQ(d.axis(0).domain.size(), 0UL);
}

TEST_F(Scalar, SaveLoadEmpty)
{
  auto f = hdf5::file::create("dummy.h5", hdf5::file::AccessFlags::TRUNCATE);
  auto g = f.root().create_group("empty");
  d.save(g);
  d.load(g);
  EXPECT_TRUE(d.empty());
}

TEST_F(Scalar, SaveLoadNonempty)
{
  d.add({{}, 3});
  d.add({{}, 5});

  auto f = hdf5::file::create("dummy.h5", hdf5::file::AccessFlags::TRUNCATE);
  auto g = f.root().create_group("nonempty");
  d.save(g);
  d.load(g);
  EXPECT_FALSE(d.empty());
  EXPECT_EQ(d.get({}), 5);
  EXPECT_EQ(d.total_count(), 2);
}

TEST_F(Scalar, SaveLoadThrow)
{
  hdf5::node::Group g;

  EXPECT_THROW(d.save(g), std::runtime_error);
  EXPECT_THROW(d.load(g), std::runtime_error);
}

TEST_F(Scalar, ExportCSV)
{
  d.add_one({});
  d.add_one({});

  std::stringstream ss;
  d.export_csv(ss);

  // TODO: could be better

  MESSAGE() << ss.str() << "\n";
}

TEST_F(Scalar, Debug)
{
  d.add_one({});
  d.add_one({});

  MESSAGE() << d.debug() << "\n";
}
