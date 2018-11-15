#include <core/time_stamp.h>
#include <gtest/gtest.h>

TEST(TimeStamp, Init)
{
  DAQuiri::TimeStamp t1{};
  ASSERT_EQ(DAQuiri::TimeBase(), t1.base());
  ASSERT_EQ(0UL, t1.native());
  ASSERT_EQ(0, t1.nanosecs());

  DAQuiri::TimeStamp t2{1};
  ASSERT_EQ(DAQuiri::TimeBase(), t2.base());
  ASSERT_EQ(1UL, t2.native());
  ASSERT_EQ(1, t2.nanosecs());

  DAQuiri::TimeStamp t3{4,DAQuiri::TimeBase(3,4)};
  ASSERT_EQ(DAQuiri::TimeBase(3,4), t3.base());
  ASSERT_EQ(4UL, t3.native());
  ASSERT_EQ(3, t3.nanosecs());

  t3.set_native(12);
  ASSERT_EQ(12UL, t3.native());
  ASSERT_EQ(9, t3.nanosecs());
}

TEST(TimeStamp, Compare)
{
  DAQuiri::TimeStamp t1{1}, t2{2}, t3{2}, t4{3};
  ASSERT_TRUE(t2.same_base(t3));
  ASSERT_EQ(t2, t3);
  ASSERT_NE(t1, t2);
  ASSERT_NE(t1, t4);
  ASSERT_NE(t3, t4);
  ASSERT_LT(t1, t2);
  ASSERT_LT(t3, t4);
  ASSERT_GT(t2, t1);
  ASSERT_GT(t4, t3);
  ASSERT_LE(t1, t3);
  ASSERT_LE(t2, t3);
  ASSERT_GE(t3, t1);
  ASSERT_GE(t3, t2);
}

TEST(TimeStamp, Arithmetic)
{
  DAQuiri::TimeStamp t1{2}, t2{1};
  ASSERT_EQ(1, t1-t2);
  ASSERT_EQ(4, t2.delayed(3).nanosecs());
  t2.delay(3);
  ASSERT_EQ(4, t2.nanosecs());
}

TEST(TimeStamp, DebugPrint)
{
  DAQuiri::TimeStamp t1{1}, t2{1, DAQuiri::TimeBase{3,7}};
  ASSERT_EQ("1", t1.debug());
  ASSERT_EQ("1x(3/7)", t2.debug());
}
