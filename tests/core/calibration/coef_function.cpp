#include "gtest_color_print.h"
#include <core/calibration/coef_function.h>

class CoefFunction : public TestBase
{
};

class MockFunction : public DAQuiri::CoefFunction
{
 public:
  // Inherit constructors
  using DAQuiri::CoefFunction::CoefFunction;

  std::string type() const override { return "MockFunction"; }
  MockFunction* clone() const override { return new MockFunction(*this); }
  double operator() (double x) const override { return 1 + x; }
  double derivative(double) const override { return 1; }

  std::string debug() const override { return ""; }
  std::string to_UTF8(int precision, bool with_rsq) const override { return ""; }
  std::string to_markup(int precision, bool with_rsq) const override { return ""; }
};

TEST_F(CoefFunction, InitDefault)
{
  MockFunction cf;
  EXPECT_TRUE(cf.coeffs().empty());
  EXPECT_DOUBLE_EQ(cf.x_offset().lower(), 0.0);
  EXPECT_DOUBLE_EQ(cf.x_offset().value(), 0.0);
  EXPECT_DOUBLE_EQ(cf.x_offset().upper(), 0.0);
  EXPECT_DOUBLE_EQ(cf.chi2(), 0.0);
}

TEST_F(CoefFunction, SetChi2)
{
  MockFunction cf;
  cf.chi2(1.5);
  EXPECT_DOUBLE_EQ(cf.chi2(), 1.5);
}

TEST_F(CoefFunction, SetXOffset)
{
  MockFunction cf;
  cf.x_offset({1, 3});
  EXPECT_DOUBLE_EQ(cf.x_offset().lower(), 1.0);
  EXPECT_DOUBLE_EQ(cf.x_offset().value(), 2.0);
  EXPECT_DOUBLE_EQ(cf.x_offset().upper(), 3.0);
}

TEST_F(CoefFunction, SetCoef)
{
  MockFunction cf;
  cf.set_coeff(0, {1, 2, 3});
  auto coefs = cf.coeffs();
  EXPECT_EQ(coefs.size(), 1u);
  EXPECT_EQ(coefs.count(0), 1u);
  auto c0 = coefs[0];
  EXPECT_EQ(c0.lower(), 1);
  EXPECT_EQ(c0.value(), 2);
  EXPECT_EQ(c0.upper(), 3);
}

TEST_F(CoefFunction, InitCoefs)
{
  MockFunction cf({5.0, 3.0}, 0.5, 1.0);
  EXPECT_DOUBLE_EQ(cf.chi2(), 1.0);
  auto coefs = cf.coeffs();
  EXPECT_EQ(coefs.size(), 2u);

  EXPECT_EQ(coefs.count(0), 1u);
  auto c0 = coefs[0];
  EXPECT_EQ(c0.lower(), 4.5);
  EXPECT_EQ(c0.value(), 5.0);
  EXPECT_EQ(c0.upper(), 5.5);

  EXPECT_EQ(coefs.count(1), 1u);
  auto c1 = coefs[1];
  EXPECT_EQ(c1.lower(), 2.5);
  EXPECT_EQ(c1.value(), 3.0);
  EXPECT_EQ(c1.upper(), 3.5);
}

TEST_F(CoefFunction, Eval)
{
  MockFunction cf;
  auto ev = cf.eval({3.0, 5.0, 7.0});
  EXPECT_EQ(ev.size(), 3u);
  EXPECT_EQ(ev[0], 4.0);
  EXPECT_EQ(ev[1], 6.0);
  EXPECT_EQ(ev[2], 8.0);
}

TEST_F(CoefFunction, Inverse)
{
  MockFunction cf;
  EXPECT_DOUBLE_EQ(cf.inverse(12.0), 11.0);
}

TEST_F(CoefFunction, Json)
{
  MockFunction cf({5.0, 3.0}, 0.5, 1.0);
  cf.x_offset({1, 3});

  nlohmann::json cfj = cf;
  MockFunction cf2 = cfj;
  EXPECT_DOUBLE_EQ(cf2.x_offset().lower(), 1.0);
  EXPECT_DOUBLE_EQ(cf2.x_offset().value(), 2.0);
  EXPECT_DOUBLE_EQ(cf2.x_offset().upper(), 3.0);

  EXPECT_DOUBLE_EQ(cf2.chi2(), 1.0);
  auto coefs = cf2.coeffs();
  EXPECT_EQ(coefs.size(), 2u);

  EXPECT_EQ(coefs.count(0), 1u);
  auto c0 = coefs[0];
  EXPECT_EQ(c0.lower(), 4.5);
  EXPECT_EQ(c0.value(), 5.0);
  EXPECT_EQ(c0.upper(), 5.5);

  EXPECT_EQ(coefs.count(1), 1u);
  auto c1 = coefs[1];
  EXPECT_EQ(c1.lower(), 2.5);
  EXPECT_EQ(c1.value(), 3.0);
  EXPECT_EQ(c1.upper(), 3.5);
}
