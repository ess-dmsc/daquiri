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
  UDoubleCorr ud2(ud);
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


TEST_F(UDoubleTest, PlusEquals)
{
  UDoubleCorr ud(2.0, 1.0);
  UDoubleCorr ud2(3.0, 0.5);

  ud += ud2;
  EXPECT_EQ(ud.mean(), 5.0);
  EXPECT_EQ(ud.deviation(), 1.5);
}

TEST_F(UDoubleTest, PlusEqualsUncorr)
{
  UDoubleUncorr ud(2.0, 3.0);
  UDoubleUncorr ud2(3.0, 4.0);

  ud += ud2;
  EXPECT_EQ(ud.mean(), 5.0);
  EXPECT_EQ(ud.deviation(), 5);
}

TEST_F(UDoubleTest, MinusEquals)
{
  UDoubleCorr ud(3.0, 1.0);
  UDoubleCorr ud2(1.0, 0.5);

  ud -= ud2;
  EXPECT_EQ(ud.mean(), 2.0);
  EXPECT_EQ(ud.deviation(), 0.5);
}

TEST_F(UDoubleTest, MinusEqualsUncorr)
{
  UDoubleUncorr ud(3.0, 3.0);
  UDoubleUncorr ud2(2.0, 4.0);

  ud -= ud2;
  EXPECT_EQ(ud.mean(), 1.0);
  EXPECT_EQ(ud.deviation(), 5);
}

TEST_F(UDoubleTest, DivEquals)
{
  UDoubleCorr ud(4.0, 2.0);

  ud /= UDoubleCorr(2.0, 1.0);
  EXPECT_EQ(ud.mean(), 2.0);
  EXPECT_EQ(ud.deviation(), 0);
}

TEST_F(UDoubleTest, DivEqualsReciprocal)
{
  auto ud = UDoubleCorr(1.0, 0.0);
  ud /= UDoubleCorr(2.0, 1.0);
  EXPECT_EQ(ud.mean(), 0.5);
  EXPECT_EQ(ud.deviation(), -0.25);
}

TEST_F(UDoubleTest, DivEqualsUncorr)
{
  UDoubleUncorr ud(8.0, 6.0);

  ud /= UDoubleUncorr(2.0, 2.0);
  EXPECT_EQ(ud.mean(), 4.0);
  EXPECT_EQ(ud.deviation(), 5.0);
}

TEST_F(UDoubleTest, DivEqualsUncorrReciprocal)
{
  UDoubleUncorr ud(1.0, 0.0);

  ud /= UDoubleUncorr(2.0, 2.0);
  EXPECT_EQ(ud.mean(), 0.5);
  EXPECT_EQ(ud.deviation(), 0.5);
}

TEST_F(UDoubleTest, TimesEquals)
{
  UDoubleCorr ud(1.0, 0.0);

  ud /= UDoubleCorr(2.0, 1.0);
  EXPECT_EQ(ud.mean(), 0.5);
  EXPECT_EQ(ud.deviation(), -0.25);

  auto ud2 = UDoubleCorr(2.0, 0.0);
  ud2 /= ud;
  EXPECT_EQ(ud2.mean(), 4.0);
  EXPECT_EQ(ud2.deviation(), 2.0);
}

TEST_F(UDoubleTest, TimesEqualsUncorr)
{
  UDoubleUncorr ud(1.0, 0.0);

  ud /= UDoubleUncorr(2.0, 2.0);
  EXPECT_EQ(ud.mean(), 0.5);
  EXPECT_EQ(ud.deviation(), 0.5);

  auto ud2 = UDoubleUncorr(4.0, 5.0);
  ud2 /= ud;
  EXPECT_EQ(ud2.mean(), 8.0);
//  EXPECT_EQ(ud2.deviation(), 6.0);
}

TEST_F(UDoubleTest, Ceiling)
{
  UDoubleUncorr ud(2.5, 1.0);
  auto ud2 = ceil(ud);
  EXPECT_EQ(ud2.mean(), 3.0);
  EXPECT_EQ(ud2.deviation(), 0.0);
}
