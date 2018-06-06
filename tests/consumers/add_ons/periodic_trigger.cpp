#include <gtest/gtest.h>
#include "periodic_trigger.h"

TEST(PeriodicTrigger, Init)
{
  DAQuiri::PeriodicTrigger pt;
  EXPECT_FALSE(pt.enabled);
  EXPECT_TRUE(pt.timeout.is_not_a_date_time());
  EXPECT_FALSE(pt.triggered);
}

TEST(PeriodicTrigger, GetSettings)
{
  DAQuiri::PeriodicTrigger pt;
  pt.enabled = true;
  pt.clock_type = DAQuiri::PeriodicTrigger::ClockType::NativeTime;
  pt.timeout = boost::posix_time::seconds(1);

  auto sets = pt.settings(3);

  auto enabled = sets.find(DAQuiri::Setting("periodic_trigger/enabled"));
  EXPECT_TRUE(enabled.get_bool());
  EXPECT_TRUE(enabled.has_index(3));

  auto cusing = sets.find(DAQuiri::Setting("periodic_trigger/clock"));
  EXPECT_EQ(cusing.selection(), DAQuiri::PeriodicTrigger::ClockType::NativeTime);
  EXPECT_TRUE(cusing.has_index(3));

  auto cat = sets.find(DAQuiri::Setting("periodic_trigger/time_out"));
  EXPECT_EQ(cat.duration(), boost::posix_time::seconds(1));
  EXPECT_TRUE(cat.has_index(3));
}

TEST(PeriodicTrigger, SetSettings)
{
  DAQuiri::PeriodicTrigger pt;
  auto sets = pt.settings(0);

  sets.set(DAQuiri::Setting::boolean("periodic_trigger/enabled", true));
  sets.set(DAQuiri::Setting::integer("periodic_trigger/clock", DAQuiri::PeriodicTrigger::ClockType::NativeTime));
  sets.set(DAQuiri::Setting("periodic_trigger/time_out", boost::posix_time::seconds(1)));

  pt.settings(sets);

  EXPECT_TRUE(pt.enabled);
  EXPECT_EQ(pt.clock_type, DAQuiri::PeriodicTrigger::ClockType::NativeTime);
  EXPECT_EQ(pt.timeout, boost::posix_time::seconds(1));
}

TEST(PeriodicTrigger, UseProducer)
{
  DAQuiri::PeriodicTrigger pt;

  DAQuiri::Status s1;
  s1.valid = true;
  s1.producer_time = boost::posix_time::microsec_clock::universal_time();
  pt.update(s1);

  auto s2 = s1;
  s2.producer_time += boost::posix_time::milliseconds(500);
  pt.update(s2);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(500));

  auto s3 = s2;
  s3.producer_time += boost::posix_time::milliseconds(1000);
  pt.update(s3);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(1500));
}

TEST(PeriodicTrigger, UseProducerPathological)
{
  DAQuiri::PeriodicTrigger pt;

  DAQuiri::Status s_bad;
  s_bad.valid = true;
  s_bad.producer_time = boost::posix_time::not_a_date_time;
  pt.update(s_bad);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(0));

  DAQuiri::Status s1;
  s1.valid = true;
  s1.producer_time = boost::posix_time::microsec_clock::universal_time();
  pt.update(s1);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(0));

  pt.update(s_bad);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(0));
}

TEST(PeriodicTrigger, UseConsumer)
{
  DAQuiri::PeriodicTrigger pt;
  pt.clock_type = DAQuiri::PeriodicTrigger::ClockType::ConsumerWallClock;

  DAQuiri::Status s1;
  s1.valid = true;
  s1.consumer_time = boost::posix_time::microsec_clock::universal_time();
  pt.update(s1);

  auto s2 = s1;
  s2.consumer_time += boost::posix_time::milliseconds(500);
  pt.update(s2);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(500));

  auto s3 = s2;
  s3.consumer_time += boost::posix_time::milliseconds(1000);
  pt.update(s3);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(1500));
}

TEST(PeriodicTrigger, UseConsumerPathological)
{
  DAQuiri::PeriodicTrigger pt;
  pt.clock_type = DAQuiri::PeriodicTrigger::ClockType::ConsumerWallClock;

  DAQuiri::Status s_bad;
  s_bad.valid = true;
  s_bad.consumer_time = boost::posix_time::not_a_date_time;
  pt.update(s_bad);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(0));

  DAQuiri::Status s1;
  s1.valid = true;
  s1.consumer_time = boost::posix_time::microsec_clock::universal_time();
  pt.update(s1);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(0));

  pt.update(s_bad);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(0));
}

TEST(PeriodicTrigger, UseNative)
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
  EXPECT_EQ(pt.recent_time_, boost::posix_time::microseconds(5));

  auto s3 = s2;
  s3.stats["native_time"] = DAQuiri::Setting::precise("native_time", 15000);
  pt.update(s3);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::microseconds(15));
}

TEST(PeriodicTrigger, UseNativePathological)
{
  DAQuiri::PeriodicTrigger pt;
  pt.clock_type = DAQuiri::PeriodicTrigger::ClockType::NativeTime;

  DAQuiri::Status s_bad;
  s_bad.valid = true;
  pt.update(s_bad);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(0));

  DAQuiri::Status s1;
  s1.valid = true;
  s1.stats["native_time"] = DAQuiri::Setting::precise("native_time", 0);
  pt.update(s1);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(0));

  pt.update(s_bad);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(0));
}

TEST(PeriodicTrigger, EvalTrigger)
{
  DAQuiri::PeriodicTrigger pt;
  pt.enabled = true;
  pt.timeout = boost::posix_time::seconds(1);

  pt.eval_trigger();
  EXPECT_FALSE(pt.triggered);

  pt.recent_time_ = boost::posix_time::milliseconds(500);
  pt.eval_trigger();
  EXPECT_FALSE(pt.triggered);

  pt.recent_time_ = boost::posix_time::milliseconds(1000);
  pt.eval_trigger();
  EXPECT_TRUE(pt.triggered);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::seconds(0));

  pt.triggered = false;
  pt.recent_time_ = boost::posix_time::milliseconds(1500);
  pt.eval_trigger();
  EXPECT_TRUE(pt.triggered);
  EXPECT_EQ(pt.recent_time_, boost::posix_time::milliseconds(500));
}