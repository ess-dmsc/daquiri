#include "digitized_value.h"
#include <gtest/gtest.h>

TEST(DigitizedValue, Equals)
{
  DAQuiri::DigitizedVal
      n1{}, n2{}, v1{1,1}, v2{1,1},
      v3{1,2}, v4{2,1};
  ASSERT_TRUE(n1 == n2);
  ASSERT_TRUE(v1 == v2);
  ASSERT_TRUE(n1 != v1);
  ASSERT_FALSE(n1 == v1);
  ASSERT_FALSE(n1 != n2);
  ASSERT_FALSE(v1 == v3);
  ASSERT_FALSE(v1 == v4);
}

TEST(DigitizedValue, UnscaledVals)
{
  DAQuiri::DigitizedVal v{10,0};
  ASSERT_EQ(10, v.val(0));
  ASSERT_EQ(10, v.val(7));
  ASSERT_EQ(10, v.val(16));
  ASSERT_EQ(10, v.val(5000));
  ASSERT_EQ(10, v.val(-20));
}

TEST(DigitizedValue, ScaledVals)
{
  DAQuiri::DigitizedVal v{16,8};
  ASSERT_EQ(0, v.val(0));
  ASSERT_EQ(8, v.val(7));
  ASSERT_EQ(16, v.val(8));
  ASSERT_EQ(32, v.val(9));
}

TEST(DigitizedValue, ReportBits)
{
  DAQuiri::DigitizedVal v{16,8};
  ASSERT_EQ(8, v.bits());
}

TEST(DigitizedValue, SetVal)
{
  DAQuiri::DigitizedVal v{0,8};
  v.set_val(12);
  ASSERT_EQ(12, v.val(v.bits()));
}

TEST(DigitizedValue, DebugPrint)
{
  DAQuiri::DigitizedVal v{1,1};
  ASSERT_EQ("1(1b)", v.debug());
}
