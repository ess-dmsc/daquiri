#include "gtest_color_print.h"
#include <core/calibration/calibration.h>
#include <core/calibration/coef_function_factory.h>
#include <core/calibration/polynomial.h>

class CalibID : public TestBase
{
};

TEST_F(CalibID, Init)
{
  DAQuiri::CalibID cid;
  EXPECT_FALSE(cid.valid());
}

TEST_F(CalibID, InitVals)
{
  DAQuiri::CalibID cid1("value1");
  EXPECT_TRUE(cid1.valid());
  EXPECT_EQ(cid1.value, "value1");

  DAQuiri::CalibID cid2("value2", "det2", "milliunit");
  EXPECT_TRUE(cid2.valid());
  EXPECT_EQ(cid2.value, "value2");
  EXPECT_EQ(cid2.detector, "det2");
  EXPECT_EQ(cid2.units, "milliunit");
}

TEST_F(CalibID, Compare)
{
  DAQuiri::CalibID c("value", "det", "unit");
  EXPECT_TRUE(c.compare({}));
  EXPECT_TRUE(c.compare({"value", "det", "unit"}));
  EXPECT_TRUE(c.compare({"", "det", ""}));
  EXPECT_TRUE(c.compare({"value", "", ""}));
  EXPECT_TRUE(c.compare({"", "", "unit"}));

  EXPECT_FALSE(c.compare({"val2", "det", "unit"}));
  EXPECT_FALSE(c.compare({"value", "det2", "unit"}));
  EXPECT_FALSE(c.compare({"value", "det", "unit2"}));
}

TEST_F(CalibID, Equals)
{
  EXPECT_EQ(DAQuiri::CalibID(), DAQuiri::CalibID());

  DAQuiri::CalibID c("value2", "det2", "milliunit");
  EXPECT_EQ(c, c);

  EXPECT_NE(c, DAQuiri::CalibID("value1", "det2", "milliunit"));
  EXPECT_NE(c, DAQuiri::CalibID("value2", "det1", "milliunit"));
  EXPECT_NE(c, DAQuiri::CalibID("value2", "det2", "unit"));
}

TEST_F(CalibID, Debug)
{
  DAQuiri::CalibID c("value", "det", "unit");
  MESSAGE() << c.debug() << "\n";
}

TEST_F(CalibID, Json)
{
  DAQuiri::CalibID c("value", "det", "unit");
  nlohmann::json j = c;
  DAQuiri::CalibID c2 = j;
  EXPECT_EQ(c, c2);
}




class FakeFunction : public DAQuiri::CoefFunction
{
 public:
  // Inherit constructors
  using DAQuiri::CoefFunction::CoefFunction;

  FakeFunction* clone() const override { return new FakeFunction(*this); }
  std::string type() const override { return "FakeFunction"; }

  std::string debug() const override { return ""; }
  std::string to_UTF8(int precision, bool with_rsq) const override
  {
    (void) precision;
    (void) with_rsq;
    return "";
  }
  std::string to_markup(int precision, bool with_rsq) const override
  {
    (void) precision;
    (void) with_rsq;
    return "";
  }
  double operator() (double x) const override { return 1 + x; }
  double derivative(double) const override { return 1; }
};

class Calibration : public TestBase
{
  virtual void TearDown() {
    DAQuiri::CoefFunctionFactory::singleton().clear();
  }
};

TEST_F(Calibration, Init)
{
  DAQuiri::Calibration c;
  EXPECT_FALSE(c.valid());
  EXPECT_FALSE(c.from().valid());
  EXPECT_FALSE(c.to().valid());
  EXPECT_FALSE(c.function());
}

TEST_F(Calibration, Created)
{
  hr_time_t c1{std::chrono::system_clock::now()};
  DAQuiri::Calibration c;
  hr_time_t c2{std::chrono::system_clock::now()};
  auto ct = c.created();

  EXPECT_LE(c1, ct);
  EXPECT_LE(ct, c2);
}

TEST_F(Calibration, FromTo)
{
  DAQuiri::Calibration c({"v1"}, {"v2"});
  EXPECT_EQ(c.from().value, "v1");
  EXPECT_EQ(c.to().value, "v2");
  EXPECT_FALSE(c.valid());
}

TEST_F(Calibration, FunctionManually)
{
  DAQUIRI_REGISTER_COEF_FUNCTION(DAQuiri::Polynomial);

  DAQuiri::Calibration c;
  auto pol = DAQuiri::CoefFunctionFactory::singleton().create_type("Polynomial");
  pol->set_coeff(0, {5.0});
  pol->set_coeff(1, {2.0});
  pol->set_coeff(2, {1.0});
  c.function(pol);
  EXPECT_EQ(c.function()->type(), "Polynomial");
  EXPECT_TRUE(c.valid());
}

TEST_F(Calibration, FunctionConvenient)
{
  DAQUIRI_REGISTER_COEF_FUNCTION(DAQuiri::Polynomial);

  DAQuiri::Calibration c;
  c.function("Polynomial" , {5.0, 2.0, 1.0});
  EXPECT_TRUE(c.valid());

  auto f = c.function();
  EXPECT_EQ(f->type(), "Polynomial");
  auto coefs = f->coeffs();
  EXPECT_EQ(coefs.size(), 3u);
  EXPECT_EQ(coefs[0].value(), 5.0);
  EXPECT_EQ(coefs[1].value(), 2.0);
  EXPECT_EQ(coefs[2].value(), 1.0);
}

TEST_F(Calibration, shallow_equals)
{
  DAQUIRI_REGISTER_COEF_FUNCTION(DAQuiri::Polynomial);

  DAQuiri::Calibration c({"v1"}, {"v2"});
  DAQuiri::Calibration c2({"v1"}, {"v2"});

  EXPECT_TRUE(c.shallow_equals(c2));
  EXPECT_TRUE(c2.shallow_equals(c));

  c.function("Polynomial" , {5.0, 2.0, 1.0});

  EXPECT_TRUE(c.shallow_equals(c2));
  EXPECT_TRUE(c2.shallow_equals(c));
}

TEST_F(Calibration, equals)
{
  DAQUIRI_REGISTER_COEF_FUNCTION(DAQuiri::Polynomial);
  DAQUIRI_REGISTER_COEF_FUNCTION(FakeFunction);

  DAQuiri::Calibration c({"v1"}, {"v2"});
  DAQuiri::Calibration c2({"v1"}, {"v2"});

  EXPECT_EQ(c, c2);
  EXPECT_EQ(c2, c);

  c.function("Polynomial" , {5.0, 2.0, 1.0});

  EXPECT_NE(c, c2);
  EXPECT_NE(c2, c);

  c.function(nullptr);
  c2.function("Polynomial" , {5.0, 2.0, 1.0});

  EXPECT_NE(c, c2);
  EXPECT_NE(c2, c);

  c.function("Polynomial" , {5.0, 2.0, 1.0});
  c2.function("FakeFunction" , {5.0, 2.0, 1.0});

  EXPECT_NE(c, c2);
  EXPECT_NE(c2, c);

  c.function("Polynomial" , {5.0, 2.0, 1.0});
  c2.function("Polynomial" , {5.0});
}

TEST_F(Calibration, transform)
{
  DAQUIRI_REGISTER_COEF_FUNCTION(DAQuiri::Polynomial);

  DAQuiri::Calibration c({"v1"}, {"v2"});
  c.function("Polynomial" , {5.0, 2.0, 1.0});

  EXPECT_DOUBLE_EQ(c.transform(1.0), 8.0);
  EXPECT_DOUBLE_EQ(c.transform(2.0), 13.0);
  EXPECT_DOUBLE_EQ(c.transform(3.0), 20.0);
}

TEST_F(Calibration, inverse)
{
  DAQUIRI_REGISTER_COEF_FUNCTION(DAQuiri::Polynomial);

  DAQuiri::Calibration c({"v1"}, {"v2"});
  c.function("Polynomial" , {5.0, 2.0, 1.0});

  EXPECT_DOUBLE_EQ(c.inverse(8.0, 0.000000001), 1.0);
  EXPECT_DOUBLE_EQ(c.inverse(13.0, 0.000000001), 2.0);
  EXPECT_DOUBLE_EQ(c.inverse(20.0, 0.000000001), 3.0);
}

TEST_F(Calibration, transform_vector_by_ref)
{
  DAQUIRI_REGISTER_COEF_FUNCTION(DAQuiri::Polynomial);
  DAQuiri::Calibration c({"v1"}, {"v2"});
  c.function("Polynomial" , {5.0, 2.0, 1.0});

  std::vector<double> vec {1.0, 2.0, 3.0};
  c.transform_by_ref(vec);
  EXPECT_DOUBLE_EQ(vec[0], 8.0);
  EXPECT_DOUBLE_EQ(vec[1], 13.0);
  EXPECT_DOUBLE_EQ(vec[2], 20.0);
}

TEST_F(Calibration, transform_vector)
{
  DAQUIRI_REGISTER_COEF_FUNCTION(DAQuiri::Polynomial);
  DAQuiri::Calibration c({"v1"}, {"v2"});
  c.function("Polynomial" , {5.0, 2.0, 1.0});

  auto vec = c.transform({1.0, 2.0, 3.0});
  EXPECT_DOUBLE_EQ(vec[0], 8.0);
  EXPECT_DOUBLE_EQ(vec[1], 13.0);
  EXPECT_DOUBLE_EQ(vec[2], 20.0);
}

TEST_F(Calibration, debug)
{
  DAQUIRI_REGISTER_COEF_FUNCTION(DAQuiri::Polynomial);
  DAQuiri::Calibration c({"v1"}, {"v2"});
  c.function("Polynomial" , {5.0, 2.0, 1.0});

  MESSAGE() << "\n" << c.debug() << "\n";
}

TEST_F(Calibration, Json)
{
  DAQUIRI_REGISTER_COEF_FUNCTION(DAQuiri::Polynomial);
  DAQuiri::Calibration c({"v1"}, {"v2"});
  c.function("Polynomial" , {5.0, 2.0, 1.0});

  nlohmann::json j = c;
  DAQuiri::Calibration c2 = j;

  EXPECT_EQ(c, c2);
  EXPECT_EQ(to_iso_extended(c.created()), to_iso_extended(c2.created()));
  // \todo direct binary compariston fails:
  // EXPECT_EQ(c.created(), c2.created());
}
