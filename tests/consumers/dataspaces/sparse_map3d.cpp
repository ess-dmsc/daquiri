#include "gtest_color_print.h"
#include <consumers/dataspaces/sparse_map3d.h>

class SparseMap3D : public TestBase
{
  protected:
    DAQuiri::SparseMap3D d;
};

TEST_F(SparseMap3D, Init)
{
  EXPECT_TRUE(d.empty());
  EXPECT_EQ(d.dimensions(), 3);
  EXPECT_EQ(d.total_count(), 0);
}

TEST_F(SparseMap3D, AddOne)
{
  d.add_one({0, 0, 0});
  EXPECT_FALSE(d.empty());
  EXPECT_EQ(d.total_count(), 1);

  d.add_one({0, 0, 0});
  EXPECT_EQ(d.total_count(), 2);
}

TEST_F(SparseMap3D, Get)
{
  EXPECT_EQ(d.get({0, 0, 0}), 0);
  d.add_one({0, 0, 0});
  EXPECT_EQ(d.get({0, 0, 0}), 1);

  EXPECT_EQ(d.get({1, 1, 1}), 0);
  d.add_one({1, 1, 1});
  EXPECT_EQ(d.get({1, 1, 1}), 1);
}

TEST_F(SparseMap3D, Add)
{
  d.add({{0, 0, 0}, 3});
  EXPECT_EQ(d.get({0, 0, 0}), 3);

  d.add({{0, 0, 0}, 5});
  EXPECT_EQ(d.get({0, 0, 0}), 8);
}

TEST_F(SparseMap3D, Clear)
{
  d.add({{0, 0, 0}, 3});
  EXPECT_EQ(d.total_count(), 3);

  d.clear();
  EXPECT_EQ(d.total_count(), 0);
  EXPECT_TRUE(d.empty());
}

TEST_F(SparseMap3D, Range)
{
  d.add_one({0, 0, 0});
  EXPECT_EQ(d.range({})->at(0).second, 1);
  EXPECT_EQ(d.range({})->at(0).first[0], 0UL);
  EXPECT_EQ(d.range({})->at(0).first[1], 0UL);
  EXPECT_EQ(d.range({})->at(0).first[2], 0UL);

  d.add_one({1, 1, 1});
  EXPECT_EQ(d.range({})->at(1).second, 1);
  EXPECT_EQ(d.range({})->at(1).first[0], 1UL);
  EXPECT_EQ(d.range({})->at(1).first[1], 1UL);
  EXPECT_EQ(d.range({})->at(1).first[2], 1UL);
}

TEST_F(SparseMap3D, Clone)
{
  d.add_one({0, 0, 0});
  d.add_one({1, 1, 1});

  auto d2 = std::shared_ptr<DAQuiri::Dataspace>(d.clone());
  EXPECT_EQ(d2->range({})->at(0).second, 1);
  EXPECT_EQ(d2->range({})->at(0).first[0], 0UL);
  EXPECT_EQ(d2->range({})->at(0).first[1], 0UL);
  EXPECT_EQ(d2->range({})->at(0).first[2], 0UL);
  EXPECT_EQ(d2->range({})->at(1).second, 1);
  EXPECT_EQ(d2->range({})->at(1).first[0], 1UL);
  EXPECT_EQ(d2->range({})->at(1).first[1], 1UL);
  EXPECT_EQ(d2->range({})->at(1).first[2], 1UL);
  EXPECT_EQ(d2->dimensions(), 3);
  EXPECT_EQ(d2->total_count(), 2);
}

TEST_F(SparseMap3D, CalcAxes)
{
  d.add_one({0, 0, 0});
  EXPECT_TRUE(d.axis(0).domain.empty());
  EXPECT_TRUE(d.axis(1).domain.empty());
  EXPECT_TRUE(d.axis(2).domain.empty());
  d.recalc_axes();
  EXPECT_EQ(d.axis(0).domain.size(), 1UL);
  EXPECT_EQ(d.axis(1).domain.size(), 1UL);
  EXPECT_EQ(d.axis(2).domain.size(), 1UL);

  d.add_one({1, 1, 1});
  EXPECT_EQ(d.axis(0).domain.size(), 1UL);
  EXPECT_EQ(d.axis(1).domain.size(), 1UL);
  EXPECT_EQ(d.axis(2).domain.size(), 1UL);
  d.recalc_axes();
  EXPECT_EQ(d.axis(0).domain.size(), 2UL);
  EXPECT_EQ(d.axis(1).domain.size(), 2UL);
  EXPECT_EQ(d.axis(2).domain.size(), 2UL);
}

TEST_F(SparseMap3D, SaveLoadEmpty)
{
  auto f = hdf5::file::create("dummy.h5", hdf5::file::AccessFlags::TRUNCATE);
  auto g = f.root().create_group("empty");
  d.save(g);
  d.load(g);
  EXPECT_TRUE(d.empty());
}

TEST_F(SparseMap3D, SaveLoadNonempty)
{
  d.add({{0, 0, 0}, 3});

  auto f = hdf5::file::create("dummy.h5", hdf5::file::AccessFlags::TRUNCATE);
  auto g = f.root().create_group("nonempty");
  d.save(g);
  d.load(g);
  EXPECT_FALSE(d.empty());
  EXPECT_EQ(d.get({0, 0, 0}), 3);
  EXPECT_EQ(d.total_count(), 3);
}

TEST_F(SparseMap3D, SaveLoadThrow)
{
  hdf5::node::Group g;

  EXPECT_THROW(d.save(g), std::runtime_error);
  EXPECT_THROW(d.load(g), std::runtime_error);
}

TEST_F(SparseMap3D, ExportCSV)
{
  d.add_one({0, 0, 0});
  d.add_one({1, 1, 1});
  d.add_one({2, 2, 2});

  std::stringstream ss;
  d.export_csv(ss);

  EXPECT_EQ(ss.str(), "x=0\n1, 0, 0;\n0, 0, 0;\n0, 0, 0;\n"
                      "x=1\n0, 0, 0;\n0, 1, 0;\n0, 0, 0;\n"
                      "x=2\n0, 0, 0;\n0, 0, 0;\n0, 0, 1;\n");
}

TEST_F(SparseMap3D, Debug)
{
  d.add_one({0, 0, 0});
  d.add_one({1, 1, 1});
  d.add_one({2, 2, 2});

  MESSAGE() << d.debug() << "\n";
}
