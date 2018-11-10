#include "gtest_color_print.h"
#include <core/calibration/uncertain.h>

class UncertainDouble : public TestBase
{
};

TEST_F(UncertainDouble, Default)
{
  DAQuiri::UncertainDouble ud;
  EXPECT_TRUE(std::isnan(ud.value()));
  EXPECT_TRUE(std::isnan(ud.sigma()));
  EXPECT_EQ(ud.sigfigs(), 1);
}

TEST_F(UncertainDouble, Construct)
{
  DAQuiri::UncertainDouble ud(1.23, 4.56, 3);
  EXPECT_EQ(ud.value(), 1.23);
  EXPECT_EQ(ud.sigma(), 4.56);
  EXPECT_EQ(ud.sigfigs(), 3);
}

TEST_F(UncertainDouble, Value)
{
  DAQuiri::UncertainDouble ud;

  ud.value(1.23);
  EXPECT_EQ(ud.value(), 1.23);

  ud.value(-1.23);
  EXPECT_EQ(ud.value(), -1.23);
}

TEST_F(UncertainDouble, Sigma)
{
  DAQuiri::UncertainDouble ud;

  ud.sigma(1.23);
  EXPECT_EQ(ud.sigma(), 1.23);

  ud.sigma(-1.23);
  EXPECT_EQ(ud.sigma(), 1.23);
}

TEST_F(UncertainDouble, SigFigs)
{
  DAQuiri::UncertainDouble ud;

  ud.sigfigs(2);
  EXPECT_EQ(ud.sigfigs(), 2);

  ud.sigfigs(0);
  EXPECT_EQ(ud.sigfigs(), 1);
}

TEST_F(UncertainDouble, Exponent)
{
  EXPECT_EQ(DAQuiri::UncertainDouble(1.0, 0.1, 0).exponent(), 0);
}

TEST_F(UncertainDouble, DeduceSigFigs)
{
  DAQuiri::UncertainDouble ud;

  ud.value(3000.0);
  ud.sigma(1.0);
  ud.deduce_sigfigs();
  EXPECT_EQ(ud.sigfigs(), 5);

  ud.value(1.0);
  ud.sigma(3000.0);
  ud.deduce_sigfigs();
  EXPECT_EQ(ud.sigfigs(), 5);

  ud.value(0.001);
  ud.sigma(0.00001);
  ud.deduce_sigfigs();
  EXPECT_EQ(ud.sigfigs(), 4);
}
