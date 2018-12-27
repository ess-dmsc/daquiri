#include "gtest_color_print.h"

#include <core/calibration/parameter.h>

class Parameter : public TestBase
{
 protected:

  DAQuiri::FitParam fp;
};

using namespace DAQuiri;

TEST_F(Parameter, Default)
{
  EXPECT_FALSE(fp.fixed());
  EXPECT_TRUE(fp.enabled());
  EXPECT_TRUE(std::isnan(fp.value()));
  EXPECT_EQ(fp.lower(), std::numeric_limits<double>::min());
  EXPECT_EQ(fp.upper(), std::numeric_limits<double>::max());
}

TEST_F(Parameter, set3equal)
{
  fp.set(1.0, 1.0, 1.0);

  EXPECT_EQ(fp.value(), 1.0);
  EXPECT_EQ(fp.lower(), 1.0);
  EXPECT_EQ(fp.upper(), 1.0);
}

TEST_F(Parameter, set3ordered)
{
  fp.set(1.0, 2.0, 3.0);

  EXPECT_EQ(fp.value(), 2.0);
  EXPECT_EQ(fp.lower(), 1.0);
  EXPECT_EQ(fp.upper(), 3.0);
}

TEST_F(Parameter, set3reversed)
{
  fp.set(3.0, 2.0, 1.0);

  EXPECT_EQ(fp.value(), 2.0);
  EXPECT_EQ(fp.lower(), 1.0);
  EXPECT_EQ(fp.upper(), 3.0);
}

TEST_F(Parameter, set3unordered)
{
  fp.set(2.0, 3.0, 1.0);

  EXPECT_EQ(fp.value(), 2.0);
  EXPECT_EQ(fp.lower(), 1.0);
  EXPECT_EQ(fp.upper(), 3.0);
}

TEST_F(Parameter, set3unordered_b)
{
  fp.set(2.0, 1.0, 3.0);

  EXPECT_EQ(fp.value(), 2.0);
  EXPECT_EQ(fp.lower(), 1.0);
  EXPECT_EQ(fp.upper(), 3.0);
}

TEST_F(Parameter, set3unordered_c)
{
  fp.set(3.0, 1.0, 2.0);

  EXPECT_EQ(fp.value(), 2.0);
  EXPECT_EQ(fp.lower(), 1.0);
  EXPECT_EQ(fp.upper(), 3.0);
}

TEST_F(Parameter, set3unordered_d)
{
  fp.set(1.0, 3.0, 2.0);

  EXPECT_EQ(fp.value(), 2.0);
  EXPECT_EQ(fp.lower(), 1.0);
  EXPECT_EQ(fp.upper(), 3.0);
}
