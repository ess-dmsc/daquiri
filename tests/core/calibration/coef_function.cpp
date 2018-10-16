#include <gtest/gtest.h>
#include <core/calibration/coef_function.h>

using namespace DAQuiri;

class MockFunction : public CoefFunction
{
  public:
    std::string type() const override { return ""; }
    std::string to_string() const override { return ""; }
    std::string to_UTF8() const override { return ""; }
    std::string to_markup() const override { return ""; }

    double eval(double) const override { return 0; }
    double derivative(double) const override { return 0; }
};

TEST(CoefFunction, Init)
{
  MockFunction cf;
  EXPECT_EQ(cf.chi2(), 0);
}
