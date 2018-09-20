#include "gtest_color_print.h"
#include <consumers/add_ons/periodic_trigger.h>

class PeriodicTrigger : public TestBase
{
};

TEST_F(PeriodicTrigger, Init)
{
  DAQuiri::PeriodicTrigger pt;
  EXPECT_FALSE(pt.enabled);
  EXPECT_EQ(pt.timeout.count(), 0);
  EXPECT_FALSE(pt.triggered);
}

TEST_F(PeriodicTrigger, GetSettings)
{
  DAQuiri::PeriodicTrigger pt;
  pt.enabled = true;
  pt.clock_type = DAQuiri::PeriodicTrigger::ClockType::NativeTime;
  pt.timeout = std::chrono::seconds(1);

  auto sets = pt.settings(3);

  auto enabled = sets.find(DAQuiri::Setting("periodic_trigger/enabled"));
  EXPECT_TRUE(enabled.get_bool());
  EXPECT_TRUE(enabled.has_index(3));

  auto cusing = sets.find(DAQuiri::Setting("periodic_trigger/clock"));
  EXPECT_EQ(cusing.selection(), DAQuiri::PeriodicTrigger::ClockType::NativeTime);
  EXPECT_TRUE(cusing.has_index(3));

  auto cat = sets.find(DAQuiri::Setting("periodic_trigger/time_out"));
  EXPECT_EQ(cat.duration(), std::chrono::seconds(1));
  EXPECT_TRUE(cat.has_index(3));
}

TEST_F(PeriodicTrigger, SetSettings)
{
  DAQuiri::PeriodicTrigger pt;
  auto sets = pt.settings(0);

  sets.set(DAQuiri::Setting::boolean("periodic_trigger/enabled", true));
  sets.set(DAQuiri::Setting::integer("periodic_trigger/clock", DAQuiri::PeriodicTrigger::ClockType::NativeTime));
  sets.set(DAQuiri::Setting("periodic_trigger/time_out", std::chrono::seconds(1)));

  pt.settings(sets);

  EXPECT_TRUE(pt.enabled);
  EXPECT_EQ(pt.clock_type, DAQuiri::PeriodicTrigger::ClockType::NativeTime);
  EXPECT_EQ(pt.timeout, std::chrono::seconds(1));
}

TEST_F(PeriodicTrigger, UseProducer)
{
  DAQuiri::PeriodicTrigger pt;

  DAQuiri::Status s1;
  s1.valid = true;
  s1.producer_time = std::chrono::system_clock::now();
  pt.update(s1);

  auto s2 = s1;
  s2.producer_time += std::chrono::milliseconds(500);
  pt.update(s2);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(500));

  auto s3 = s2;
  s3.producer_time += std::chrono::milliseconds(1000);
  pt.update(s3);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(1500));
}

/*
TEST_F(PeriodicTrigger, UseProducerPathological)
{
  DAQuiri::PeriodicTrigger pt;

  DAQuiri::Status s_bad;
  s_bad.valid = true;
  pt.update(s_bad);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(0));

  DAQuiri::Status s1;
  s1.valid = true;
  s1.producer_time = std::chrono::system_clock::now();
  pt.update(s1);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(0)) << to_simple(pt.recent_time_);

  pt.update(s_bad);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(0)) << to_simple(pt.recent_time_);
}
*/

TEST_F(PeriodicTrigger, UseConsumer)
{
  DAQuiri::PeriodicTrigger pt;
  pt.clock_type = DAQuiri::PeriodicTrigger::ClockType::ConsumerWallClock;

  DAQuiri::Status s1;
  s1.valid = true;
  s1.consumer_time = std::chrono::system_clock::now();
  pt.update(s1);

  auto s2 = s1;
  s2.consumer_time += std::chrono::milliseconds(500);
  pt.update(s2);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(500));

  auto s3 = s2;
  s3.consumer_time += std::chrono::milliseconds(1000);
  pt.update(s3);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(1500));
}

/*
TEST_F(PeriodicTrigger, UseConsumerPathological)
{
  DAQuiri::PeriodicTrigger pt;
  pt.clock_type = DAQuiri::PeriodicTrigger::ClockType::ConsumerWallClock;

  DAQuiri::Status s_bad;
  s_bad.valid = true;
  s_bad.consumer_time = std::chrono::time_point<std::chrono::system_clock>();
  pt.update(s_bad);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(0));

  DAQuiri::Status s1;
  s1.valid = true;
  s1.consumer_time = std::chrono::system_clock::now();
  pt.update(s1);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(0));

  pt.update(s_bad);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(0));
}
*/

TEST_F(PeriodicTrigger, UseNative)
{
  DAQuiri::PeriodicTrigger pt;
  pt.clock_type = DAQuiri::PeriodicTrigger::ClockType::NativeTime;

  DAQuiri::Status s1;
  s1.valid = true;
  s1.stats["native_time"] = DAQuiri::Setting::precise("native_time", 0);
  pt.update(s1);

  auto s2 = s1;
  s2.stats["native_time"] = DAQuiri::Setting::precise("native_time", 5000);
  pt.update(s2);
  EXPECT_EQ(pt.recent_time_, std::chrono::microseconds(5));

  auto s3 = s2;
  s3.stats["native_time"] = DAQuiri::Setting::precise("native_time", 15000);
  pt.update(s3);
  EXPECT_EQ(std::chrono::duration_cast<std::chrono::microseconds>(pt.recent_time_),
      std::chrono::microseconds(15));
}

TEST_F(PeriodicTrigger, UseNativePathological)
{
  DAQuiri::PeriodicTrigger pt;
  pt.clock_type = DAQuiri::PeriodicTrigger::ClockType::NativeTime;

  DAQuiri::Status s_bad;
  s_bad.valid = true;
  pt.update(s_bad);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(0));

  DAQuiri::Status s1;
  s1.valid = true;
  s1.stats["native_time"] = DAQuiri::Setting::precise("native_time", 0);
  pt.update(s1);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(0));

  pt.update(s_bad);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(0));
}

TEST_F(PeriodicTrigger, EvalTrigger)
{
  DAQuiri::PeriodicTrigger pt;
  pt.enabled = true;
  pt.timeout = std::chrono::seconds(1);

  pt.eval_trigger();
  EXPECT_FALSE(pt.triggered);

  pt.recent_time_ = std::chrono::milliseconds(500);
  pt.eval_trigger();
  EXPECT_FALSE(pt.triggered);

  pt.recent_time_ = std::chrono::milliseconds(1000);
  pt.eval_trigger();
  EXPECT_TRUE(pt.triggered);
  EXPECT_EQ(pt.recent_time_, std::chrono::seconds(0));

  pt.triggered = false;
  pt.recent_time_ = std::chrono::milliseconds(1500);
  pt.eval_trigger();
  EXPECT_TRUE(pt.triggered);
  EXPECT_EQ(pt.recent_time_, std::chrono::milliseconds(500));
}