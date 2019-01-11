#include "gtest_color_print.h"
#include <core/calibration/calibration.h>

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



class Calibration : public TestBase
{
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
}

