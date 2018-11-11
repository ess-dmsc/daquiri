#include "gtest_color_print.h"
#include <core/calibration/udouble.h>

class UDoubleTest : public TestBase
{
};

TEST_F(UDoubleTest, ConstructCorrelatedPositive)
{
  UDoubleCorr ud(2.0, 1.0);
  EXPECT_EQ(ud.mean(), 2.0);
  EXPECT_EQ(ud.deviation(), 1.0);
}

TEST_F(UDoubleTest, ConstructCorrelatedNegative)
{
  UDoubleCorr ud(2.0, -1.0);
  EXPECT_EQ(ud.mean(), 2.0);
  EXPECT_EQ(ud.deviation(), -1.0);
}

TEST_F(UDoubleTest, ConstructUncorrelatedPositive)
{
  UDoubleUncorr ud(2.0, 1.0);
  EXPECT_EQ(ud.mean(), 2.0);
  EXPECT_EQ(ud.deviation(), 1.0);
}

TEST_F(UDoubleTest, UncorrelatedNegativeThrows)
{
  EXPECT_ANY_THROW(UDoubleUncorr(2.0, -1.0));
}

TEST_F(UDoubleTest, Copy)
{
  UDoubleCorr ud(2.0, -1.0);
  UDoubleCorr ud2 = ud;
  EXPECT_EQ(ud2.mean(), 2.0);
  EXPECT_EQ(ud2.deviation(), -1.0);
}

TEST_F(UDoubleTest, UnaryPlus)
{
  UDoubleCorr ud(2.0, -1.0);
  UDoubleCorr ud2 = +ud;
  EXPECT_EQ(ud2.mean(), 2.0);
  EXPECT_EQ(ud2.deviation(), -1.0);
}

TEST_F(UDoubleTest, UnaryNegateCorrelated)
{
  UDoubleCorr ud(2.0, -1.0);
  UDoubleCorr ud2 = -ud;
  EXPECT_EQ(ud2.mean(), -2.0);
  EXPECT_EQ(ud2.deviation(), 1.0);
}

TEST_F(UDoubleTest, UnaryNegateUncorrelated)
{
  UDoubleUncorr ud(2.0, 1.0);
  UDoubleUncorr ud2 = -ud;
  EXPECT_EQ(ud2.mean(), -2.0);
  EXPECT_EQ(ud2.deviation(), 1.0);
}

TEST_F(UDoubleTest, Ceiling)
{
  UDoubleUncorr ud(2.5, 1.0);
  UDoubleUncorr ud2 = ceil(ud);
  EXPECT_EQ(ud2.mean(), 3.0);
  EXPECT_EQ(ud2.deviation(), 0.0);
}
