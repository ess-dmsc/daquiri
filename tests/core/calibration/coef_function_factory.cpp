#include "gtest_color_print.h"
#include <core/calibration/coef_function_factory.h>

class CoefFunctionFactory : public TestBase
{
  virtual void TearDown() {
    DAQuiri::CoefFunctionFactory::singleton().clear();
  }
};

class FakeFunction : public DAQuiri::CoefFunction
{
 public:
  // Inherit constructors
  using DAQuiri::CoefFunction::CoefFunction;

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

class Function1 : public FakeFunction
{
 public:
  Function1* clone() const override { return new Function1(*this); }
  std::string type() const override { return "Function1"; }
};

class Function2 : public FakeFunction
{
 public:
  Function2* clone() const override { return new Function2(*this); }
  std::string type() const override { return "Function2"; }
};

class Function3 : public FakeFunction
{
 public:
  Function3* clone() const override { return new Function3(*this); }
  std::string type() const override { return "Function3"; }
};


TEST_F(CoefFunctionFactory, Singleton)
{
  auto& a = DAQuiri::CoefFunctionFactory::singleton();
  auto& b = DAQuiri::CoefFunctionFactory::singleton();
  EXPECT_EQ(&a, &b);
}


TEST_F(CoefFunctionFactory, types)
{
  auto& cf = DAQuiri::CoefFunctionFactory::singleton();

  EXPECT_TRUE(cf.types().empty());
  EXPECT_EQ(cf.types().size(), 0UL);

  DAQUIRI_REGISTER_COEF_FUNCTION(Function1);
  EXPECT_EQ(cf.types().size(), 1UL);

  DAQUIRI_REGISTER_COEF_FUNCTION(Function2);
  EXPECT_EQ(cf.types().size(), 2UL);

  DAQUIRI_REGISTER_COEF_FUNCTION(Function3);
  EXPECT_EQ(cf.types().size(), 3UL);

  cf.clear();
  EXPECT_EQ(cf.types().size(), 0UL);
  EXPECT_TRUE(cf.types().empty());
}

TEST_F(CoefFunctionFactory, create_type)
{
  auto& cf = DAQuiri::CoefFunctionFactory::singleton();
  DAQUIRI_REGISTER_COEF_FUNCTION(Function1);
  DAQUIRI_REGISTER_COEF_FUNCTION(Function2);

  EXPECT_FALSE(cf.create_type("bad_id"));
  EXPECT_EQ(cf.create_type("Function1")->type(), "Function1");
  EXPECT_EQ(cf.create_type("Function2")->type(), "Function2");
}

TEST_F(CoefFunctionFactory, create_copy)
{
  auto& cf = DAQuiri::CoefFunctionFactory::singleton();
  DAQUIRI_REGISTER_COEF_FUNCTION(Function1);

  auto c1 = cf.create_type("Function1");
  auto c2 = cf.create_copy(c1);

  EXPECT_EQ(c2->type(), "Function1");
  EXPECT_NE(c1.get(), c2.get());

  EXPECT_FALSE(cf.create_copy(nullptr));
}

TEST_F(CoefFunctionFactory, create_from_json)
{
  auto& cf = DAQuiri::CoefFunctionFactory::singleton();
  DAQUIRI_REGISTER_COEF_FUNCTION(Function1);

  auto c1 = cf.create_type("Function1");
  c1->set_coeff(1, {1,2,3});
  c1->set_coeff(5, {6,7,8});

  nlohmann::json j = *c1;

  auto c2 = cf.create_from_json(j);

  EXPECT_EQ(c2->type(), "Function1");
  EXPECT_NE(c1.get(), c2.get());
  EXPECT_EQ(c1->coeffs(), c2->coeffs());
}
