#include "gtest_color_print.h"
#include <core/util/lexical_extensions.h>

class LexicalExtensions : public TestBase
{
};

TEST_F(LexicalExtensions, IsNumber)
{
  EXPECT_TRUE(is_number("123"));
  EXPECT_TRUE(is_number(" 123"));
  EXPECT_TRUE(is_number("123 "));
  EXPECT_TRUE(is_number("1.23"));
  EXPECT_TRUE(is_number("1.23e100"));
  EXPECT_TRUE(is_number("-1.23"));

  EXPECT_FALSE(is_number(""));
  EXPECT_FALSE(is_number("   "));
  EXPECT_FALSE(is_number("ghb"));
  EXPECT_FALSE(is_number("+-"));
  EXPECT_FALSE(is_number("3a2"));

  EXPECT_TRUE(is_number("0x3a2"));
}

TEST_F(LexicalExtensions, FloatDeconstructed)
{
  FloatDeconstructed fd;

  fd.parse("");
  EXPECT_EQ(fd.sign, "");
  EXPECT_EQ(fd.mantissa, "");
  EXPECT_EQ(fd.exponent, "");

  fd.parse("   ");
  EXPECT_EQ(fd.sign, "");
  EXPECT_EQ(fd.mantissa, "");
  EXPECT_EQ(fd.exponent, "");

  fd.parse("123");
  EXPECT_EQ(fd.sign, "");
  EXPECT_EQ(fd.mantissa, "123");
  EXPECT_EQ(fd.exponent, "");

  fd.parse("-123");
  EXPECT_EQ(fd.sign, "-");
  EXPECT_EQ(fd.mantissa, "123");
  EXPECT_EQ(fd.exponent, "");

  fd.parse("+123");
  EXPECT_EQ(fd.sign, "+");
  EXPECT_EQ(fd.mantissa, "123");
  EXPECT_EQ(fd.exponent, "");

  fd.parse("3e12");
  EXPECT_EQ(fd.sign, "");
  EXPECT_EQ(fd.mantissa, "3");
  EXPECT_EQ(fd.exponent, "12");

  fd.parse("-3e12");
  EXPECT_EQ(fd.sign, "-");
  EXPECT_EQ(fd.mantissa, "3");
  EXPECT_EQ(fd.exponent, "12");

  fd.parse("+3e12");
  EXPECT_EQ(fd.sign, "+");
  EXPECT_EQ(fd.mantissa, "3");
  EXPECT_EQ(fd.exponent, "12");

  fd.parse("3e-12");
  EXPECT_EQ(fd.sign, "");
  EXPECT_EQ(fd.mantissa, "3");
  EXPECT_EQ(fd.exponent, "-12");

  fd.parse("3E-12");
  EXPECT_EQ(fd.sign, "");
  EXPECT_EQ(fd.mantissa, "3");
  EXPECT_EQ(fd.exponent, "-12");

  // is this ok?
  fd.parse("zef");
  EXPECT_EQ(fd.sign, "");
  EXPECT_EQ(fd.mantissa, "z");
  EXPECT_EQ(fd.exponent, "f");
}

TEST_F(LexicalExtensions, SigDigits)
{
  EXPECT_EQ(sig_digits(""), 0);
  EXPECT_EQ(sig_digits("\n\t "), 0);
  EXPECT_EQ(sig_digits("123"), 3);
  EXPECT_EQ(sig_digits("1230"), 3);
  EXPECT_EQ(sig_digits("1230."), 4);
  EXPECT_EQ(sig_digits("123.0"), 4);
  EXPECT_EQ(sig_digits("0.01"), 1);
  EXPECT_EQ(sig_digits("0.00010"), 2);
  EXPECT_EQ(sig_digits("0.00010e12"), 2);
  EXPECT_EQ(sig_digits("0.00010e-12"), 2);
  EXPECT_EQ(sig_digits("-0.00010e12"), 2);
}

TEST_F(LexicalExtensions, OrderOf)
{
  EXPECT_EQ(order_of(0), 0);
  EXPECT_EQ(order_of(2), 0);
  EXPECT_EQ(order_of(10), 1);
  EXPECT_EQ(order_of(100), 2);
  EXPECT_EQ(order_of(0.1), -1);
  EXPECT_EQ(order_of(0.001), -3);

  EXPECT_EQ(order_of(2e2), 2);
  EXPECT_EQ(order_of(200e2), 4);
  EXPECT_EQ(order_of(2e-2), -2);
  EXPECT_EQ(order_of(200e-2), 0);
  EXPECT_EQ(order_of(0.02e-2), -4);
}

TEST_F(LexicalExtensions, GetPrecision)
{
  EXPECT_EQ(get_precision("1"), 1.0);
  // \todo untested
}
