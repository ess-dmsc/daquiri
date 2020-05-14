#include <core/TimeBase.h>
#include <gtest/gtest.h>

TEST(TimeBase, Init)
{
  ASSERT_ANY_THROW(DAQuiri::TimeBase(1, 0));
  ASSERT_ANY_THROW(DAQuiri::TimeBase(0, 1));

  DAQuiri::TimeBase t1{};
  ASSERT_EQ(1, t1.multiplier());
  ASSERT_EQ(1, t1.divider());

  DAQuiri::TimeBase t2{2,4};
  ASSERT_EQ(1, t2.multiplier());
  ASSERT_EQ(2, t2.divider());
}

TEST(TimeBase, Convert)
{
  DAQuiri::TimeBase
      t1{}, t2{1,4}, t3{4,1};
  ASSERT_EQ(1, t1.to_nanosec(1));
  ASSERT_EQ(0.25, t2.to_nanosec(1));
  ASSERT_EQ(4, t3.to_nanosec(1));
  ASSERT_EQ(1, t1.to_native(1));
  ASSERT_EQ(4, t2.to_native(1));
  ASSERT_EQ(0.25, t3.to_native(1));
}

TEST(TimeBase, Compare)
{
  DAQuiri::TimeBase
      t1{}, t2{}, t3{4,1};
  ASSERT_TRUE(t1 == t2);
  ASSERT_FALSE(t1 != t2);
  ASSERT_TRUE(t1 != t3);
  ASSERT_FALSE(t1 == t3);
}

TEST(TimeBase, Common)
{
  DAQuiri::TimeBase
      t1{}, t2{1,3}, t3{1,2};
  ASSERT_EQ(t2, DAQuiri::TimeBase::common(t1, t2));
  ASSERT_EQ(t3, DAQuiri::TimeBase::common(t1, t3));
  ASSERT_EQ(DAQuiri::TimeBase(1,6),
            DAQuiri::TimeBase::common(t2, t3));
  DAQuiri::TimeBase t4{3,7}, t5{7,3};
  ASSERT_EQ(DAQuiri::TimeBase(1,21),
            DAQuiri::TimeBase::common(t4, t5));
  DAQuiri::TimeBase t6{3,7}, t7{6,3};
  ASSERT_EQ(DAQuiri::TimeBase(1,7),
            DAQuiri::TimeBase::common(t6, t7));
}

TEST(TimeBase, DebugPrint)
{
  DAQuiri::TimeBase t1{}, t2{3,7};
  ASSERT_EQ("", t1.debug());
  ASSERT_EQ("x(3/7)", t2.debug());
}
