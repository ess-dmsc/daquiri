#include "gtest_color_print.h"

#include <core/calibration/parameter.h>

class Parameter : public TestBase
{
};

TEST_F(Parameter, Default)
{
  DAQuiri::Parameter fp;
  EXPECT_FALSE(fp.fixed());
  EXPECT_TRUE(std::isnan(fp.value()));
  EXPECT_EQ(fp.lower(), -1 * std::numeric_limits<double>::infinity());
  EXPECT_EQ(fp.upper(), std::numeric_limits<double>::infinity());
}

TEST_F(Parameter, set3unordered)
{
  DAQuiri::Parameter fp;
  fp.set(2.0, 3.0, 1.0);

  EXPECT_EQ(fp.value(), 2.0);
  EXPECT_EQ(fp.lower(), 1.0);
  EXPECT_EQ(fp.upper(), 3.0);
}

TEST_F(Parameter, construct3unordered)
{
  DAQuiri::Parameter fp(2.0, 3.0, 1.0);

  EXPECT_EQ(fp.value(), 2.0);
  EXPECT_EQ(fp.lower(), 1.0);
  EXPECT_EQ(fp.upper(), 3.0);
}

TEST_F(Parameter, Constrain)
{
  DAQuiri::Parameter fp;

  fp.constrain(3.0, 1.0);

  EXPECT_EQ(fp.value(), 2.0);
  EXPECT_EQ(fp.lower(), 1.0);
  EXPECT_EQ(fp.upper(), 3.0);
}

TEST_F(Parameter, ConstructFromBounds)
{
  DAQuiri::Parameter fp(3.0, 1.0);

  EXPECT_EQ(fp.value(), 2.0);
  EXPECT_EQ(fp.lower(), 1.0);
  EXPECT_EQ(fp.upper(), 3.0);
}


TEST_F(Parameter, SetValueUnbounded)
{
  DAQuiri::Parameter fp;

  fp.value(7.0);
  EXPECT_EQ(fp.value(), 7.0);

  fp.value(-42.0);
  EXPECT_EQ(fp.value(), -42.0);
}

TEST_F(Parameter, ConstructFromValue)
{
  DAQuiri::Parameter fp(7.0);

  EXPECT_EQ(fp.value(), 7.0);
  EXPECT_EQ(fp.lower(), -1 * std::numeric_limits<double>::infinity());
  EXPECT_EQ(fp.upper(), std::numeric_limits<double>::infinity());
}

TEST_F(Parameter, SetValueExpandBounds)
{
  DAQuiri::Parameter fp(0,0);

  fp.value(7.0);
  EXPECT_EQ(fp.value(), 7.0);
  EXPECT_EQ(fp.lower(), 0.0);
  EXPECT_EQ(fp.upper(), 7.0);

  fp.value(-42.0);
  EXPECT_EQ(fp.value(), -42.0);
  EXPECT_EQ(fp.lower(), -42.0);
  EXPECT_EQ(fp.upper(), 7.0);

  fp.value(1.0);
  EXPECT_EQ(fp.value(), 1.0);
  EXPECT_EQ(fp.lower(), -42.0);
  EXPECT_EQ(fp.upper(), 7.0);
}

TEST_F(Parameter, ImplicitlyFixed)
{
  EXPECT_FALSE(DAQuiri::Parameter().implicitly_fixed());
  EXPECT_FALSE(DAQuiri::Parameter(2,3,3).implicitly_fixed());
  EXPECT_FALSE(DAQuiri::Parameter(2,2,3).implicitly_fixed());
  EXPECT_TRUE(DAQuiri::Parameter(3,3).implicitly_fixed());
}

TEST_F(Parameter, EqualBounds)
{
  EXPECT_TRUE(DAQuiri::Parameter().equal_bounds(DAQuiri::Parameter()));
  EXPECT_TRUE(DAQuiri::Parameter(1).equal_bounds(DAQuiri::Parameter()));
  EXPECT_TRUE(DAQuiri::Parameter(1).equal_bounds(DAQuiri::Parameter(3)));
  EXPECT_TRUE(DAQuiri::Parameter(1,3).equal_bounds(DAQuiri::Parameter(3,1)));
  EXPECT_TRUE(DAQuiri::Parameter(1,3,2).equal_bounds(DAQuiri::Parameter(3,1,2.5)));

  EXPECT_FALSE(DAQuiri::Parameter(1,3).equal_bounds(DAQuiri::Parameter()));
  EXPECT_FALSE(DAQuiri::Parameter(1,3).equal_bounds(DAQuiri::Parameter(4,5)));
}

TEST_F(Parameter, ToString)
{
  EXPECT_EQ(DAQuiri::Parameter().to_string(), "nan[-inf,inf]");
}

TEST_F(Parameter, json)
{
  DAQuiri::Parameter fp(2.0, 3.0, 1.0);

  nlohmann::json fpj = fp;
  DAQuiri::Parameter fp2 = fpj;

  EXPECT_EQ(fp2.value(), 2.0);
  EXPECT_EQ(fp2.lower(), 1.0);
  EXPECT_EQ(fp2.upper(), 3.0);
}

TEST_F(Parameter, Compare)
{
  EXPECT_EQ(DAQuiri::Parameter(2.0, 3.0, 1.0), DAQuiri::Parameter(1.0, 2.0, 3.0));
  EXPECT_FALSE(DAQuiri::Parameter(1.0, 2.0, 3.0) < DAQuiri::Parameter(1.0, 2.0, 3.0));

  EXPECT_NE(DAQuiri::Parameter(1.0, 2.0, 3.0), DAQuiri::Parameter(1.0, 2.5, 3.0));
  EXPECT_LT(DAQuiri::Parameter(1.0, 2.0, 3.0), DAQuiri::Parameter(1.0, 2.5, 3.0));

  EXPECT_NE(DAQuiri::Parameter(1.0, 2.0, 3.0), DAQuiri::Parameter(1.1, 2.0, 3.0));
  EXPECT_LT(DAQuiri::Parameter(1.0, 2.0, 3.0), DAQuiri::Parameter(1.1, 2.0, 3.0));

  EXPECT_NE(DAQuiri::Parameter(1.0, 2.0, 3.0), DAQuiri::Parameter(1.0, 2.0, 3.1));
  EXPECT_LT(DAQuiri::Parameter(1.0, 2.0, 3.0), DAQuiri::Parameter(1.0, 2.0, 3.1));
}
