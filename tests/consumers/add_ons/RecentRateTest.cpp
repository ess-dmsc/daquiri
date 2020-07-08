#include "gtest_color_print.h"
#include <consumers/add_ons/RecentRate.h>

class RecentRate : public TestBase
{
  protected:
    virtual void SetUp()
    {
      s.valid = true;
    }

    DAQuiri::RecentRate rr;
    DAQuiri::Status s;
};

TEST_F(RecentRate, DefaultConstructor)
{
  EXPECT_TRUE(rr.divisor_clock.empty());
  EXPECT_EQ(rr.previous_count, 0);
  EXPECT_FALSE(rr.previous_status.valid);
  EXPECT_EQ(rr.current_rate, 0);
}

TEST_F(RecentRate, ConvenienceConstructor)
{
  rr = DAQuiri::RecentRate("live_time");
  EXPECT_EQ(rr.divisor_clock, "live_time");
  EXPECT_EQ(rr.previous_count, 0);
  EXPECT_FALSE(rr.previous_status.valid);
  EXPECT_EQ(rr.current_rate, 0);
}

TEST_F(RecentRate, GetSettings)
{
  rr.divisor_clock = "live_time";

  auto sets = rr.settings(3);

  auto divc = sets.find(DAQuiri::Setting("divisor_clock"));
  EXPECT_EQ(divc.get_text(), "live_time");
  EXPECT_TRUE(divc.has_index(3));
}

TEST_F(RecentRate, SetSettings)
{
  auto sets = rr.settings(0);

  sets.set(DAQuiri::Setting::text("divisor_clock", "live_time"));

  rr.settings(sets);

  EXPECT_EQ(rr.divisor_clock, "live_time");
}

TEST_F(RecentRate, OneStatusResultsZero)
{
  rr.divisor_clock = "live_time";

  s.stats["live_time"] = DAQuiri::Setting::floating("live_time", 1 * pow(10, 9));

  rr.update(s, 1000);

  EXPECT_EQ(rr.current_rate, 0);
}

TEST_F(RecentRate, IdenticalStatusesResultZero)
{
  rr.divisor_clock = "live_time";

  s.stats["live_time"] = DAQuiri::Setting::floating("live_time", 1 * pow(10, 9));
  rr.update(s, 0);
  rr.update(s, 1000);

  EXPECT_EQ(rr.current_rate, 0);
}

TEST_F(RecentRate, OneSecondDifference)
{
  rr.divisor_clock = "live_time";

  s.stats["live_time"] = DAQuiri::Setting::floating("live_time", 1 * pow(10, 9));
  rr.update(s, 0);

  s.stats["live_time"] = DAQuiri::Setting::floating("live_time", 2 * pow(10, 9));
  rr.update(s, 1000);

  EXPECT_EQ(rr.current_rate, 1000);
}

TEST_F(RecentRate, SubsequentUpdates)
{
  rr.divisor_clock = "live_time";

  s.stats["live_time"] = DAQuiri::Setting::floating("live_time", 1 * pow(10, 9));
  rr.update(s, 0);

  s.stats["live_time"] = DAQuiri::Setting::floating("live_time", 2 * pow(10, 9));
  rr.update(s, 1000);

  s.stats["live_time"] = DAQuiri::Setting::floating("live_time", 3 * pow(10, 9));
  rr.update(s, 1500);

  EXPECT_EQ(rr.current_rate, 500);
}

TEST_F(RecentRate, NoDifferenceInCounts)
{
  rr.divisor_clock = "live_time";

  s.stats["live_time"] = DAQuiri::Setting::floating("live_time", 1 * pow(10, 9));
  rr.update(s, 1000);

  s.stats["live_time"] = DAQuiri::Setting::floating("live_time", 2 * pow(10, 9));
  rr.update(s, 1000);

  EXPECT_EQ(rr.current_rate, 0);
}

TEST_F(RecentRate, ReturnsSettingValue)
{
  rr.divisor_clock = "live_time";

  s.stats["live_time"] = DAQuiri::Setting::floating("live_time", 1 * pow(10, 9));
  rr.update(s, 0);

  s.stats["live_time"] = DAQuiri::Setting::floating("live_time", 2 * pow(10, 9));
  auto ret = rr.update(s, 1000);

  EXPECT_EQ(ret.id(), "recent_live_time_rate");
  EXPECT_EQ(ret.get_number(), 1000);
}
