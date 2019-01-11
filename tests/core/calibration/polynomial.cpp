#include "gtest_color_print.h"
#include <core/calibration/polynomial.h>

class Polynomial : public TestBase
{
};

TEST_F(Polynomial, InitDefault)
{
  DAQuiri::Polynomial cf;
  EXPECT_TRUE(cf.coeffs().empty());
  EXPECT_DOUBLE_EQ(cf.x_offset().lower(), 0.0);
  EXPECT_DOUBLE_EQ(cf.x_offset().value(), 0.0);
  EXPECT_DOUBLE_EQ(cf.x_offset().upper(), 0.0);
  EXPECT_DOUBLE_EQ(cf.chi2(), 0.0);
}

TEST_F(Polynomial, SetChi2)
{
  DAQuiri::Polynomial cf;
  cf.chi2(1.5);
  EXPECT_DOUBLE_EQ(cf.chi2(), 1.5);
}

TEST_F(Polynomial, SetXOffset)
{
  DAQuiri::Polynomial cf;
  cf.x_offset({1, 3});
  EXPECT_DOUBLE_EQ(cf.x_offset().lower(), 1.0);
  EXPECT_DOUBLE_EQ(cf.x_offset().value(), 2.0);
  EXPECT_DOUBLE_EQ(cf.x_offset().upper(), 3.0);
}

TEST_F(Polynomial, SetCoef)
{
  DAQuiri::Polynomial cf;
  cf.set_coeff(0, {1, 2, 3});
  auto coefs = cf.coeffs();
  EXPECT_EQ(coefs.size(), 1u);
  EXPECT_EQ(coefs.count(0), 1u);
  auto c0 = coefs[0];
  EXPECT_EQ(c0.lower(), 1);
  EXPECT_EQ(c0.value(), 2);
  EXPECT_EQ(c0.upper(), 3);
}

TEST_F(Polynomial, InitCoefs)
{
  DAQuiri::Polynomial cf({5.0, 3.0}, 0.5, 1.0);
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

TEST_F(Polynomial, Eval)
{
  DAQuiri::Polynomial cf{{5.0, 2.0, 1.0}, 0, 0};
  auto ev = cf.eval({1.0, 2.0, 3.0});
  EXPECT_EQ(ev.size(), 3u);
  EXPECT_EQ(ev[0], 8.0);
  EXPECT_EQ(ev[1], 13.0);
  EXPECT_EQ(ev[2], 20.0);
}

TEST_F(Polynomial, Inverse)
{
  DAQuiri::Polynomial cf{{5.0, 2.0, 1.0}, 0, 0};
  EXPECT_DOUBLE_EQ(cf.inverse(8.0, 0.000000001), 1.0);
  EXPECT_DOUBLE_EQ(cf.inverse(13.0, 0.000000001), 2.0);
  EXPECT_DOUBLE_EQ(cf.inverse(20.0, 0.000000001), 3.0);
}

TEST_F(Polynomial, Debug)
{
  DAQuiri::Polynomial cf{{5.0, 2.0, 1.0}, 0, 0};
  MESSAGE() << cf.debug() << "\n";
}

TEST_F(Polynomial, UTF8)
{
  DAQuiri::Polynomial cf{{5.0, 2.0, 1.0}, 0, 0};
  MESSAGE() << cf.to_UTF8(3, true) << "\n";
}

TEST_F(Polynomial, Markup)
{
  DAQuiri::Polynomial cf{{5.0, 2.0, 1.0}, 0, 0};
  MESSAGE() << cf.to_markup(3, true) << "\n";
}


