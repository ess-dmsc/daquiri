#include "hit.h"
#include <gtest/gtest.h>

TEST(Hit, Init)
{
  DAQuiri::Hit h;
  ASSERT_EQ(-1, h.source_channel());
  ASSERT_EQ(DAQuiri::TimeStamp(), h.timestamp());
  ASSERT_EQ(0, h.value_count());
  ASSERT_EQ(0, h.trace_count());
  EXPECT_EQ("[ch-1|t0]", h.debug());

  DAQuiri::HitModel hm;
  hm.timebase = DAQuiri::TimeBase(14,10);
  hm.add_value("energy", 16);
  hm.add_trace("wave", {3});
  DAQuiri::Hit h2(2, hm);
  ASSERT_EQ(2, h2.source_channel());
  ASSERT_EQ(DAQuiri::TimeBase(7,5),
            h2.timestamp().base());
  ASSERT_EQ(1, h2.value_count());
  ASSERT_EQ(1, h2.trace_count());
  EXPECT_EQ("[ch2|t0x(7/5)|ntraces=1 0(16b)]", h2.debug());
}

TEST(Hit, Value)
{
  DAQuiri::HitModel hm;
  hm.timebase = DAQuiri::TimeBase(14,10);
  hm.add_value("energy", 16);
  DAQuiri::Hit h(2, hm);
  ASSERT_ANY_THROW(h.value(2));
  ASSERT_NO_THROW(h.set_value(0,42));
  ASSERT_EQ(42, h.value(0).val(16));
  EXPECT_EQ("[ch2|t0x(7/5) 42(16b)]", h.debug());
}

TEST(Hit, Trace)
{
  DAQuiri::HitModel hm;
  hm.timebase = DAQuiri::TimeBase(14,10);
  hm.add_trace("wave", {3});
  DAQuiri::Hit h(2, hm);
  ASSERT_ANY_THROW(h.trace(2));
  ASSERT_NO_THROW(h.set_trace(0, {3,6,9}));
  ASSERT_EQ(std::vector<uint16_t>({3,6,9}),
            h.trace(0));
  EXPECT_EQ("[ch2|t0x(7/5)|ntraces=1]", h.debug());
}

TEST(Hit, Time)
{
  DAQuiri::HitModel hm;
  hm.timebase = DAQuiri::TimeBase(2,1);
  DAQuiri::Hit h(2, hm);
  ASSERT_EQ(DAQuiri::TimeBase(2,1), h.timestamp().base());
  h.set_native_time(10);
  ASSERT_EQ(20, h.timestamp().nanosecs());
  h.set_timestamp(DAQuiri::TimeStamp(10, DAQuiri::TimeBase(7,5)));
  ASSERT_EQ(14, h.timestamp().nanosecs());
  EXPECT_EQ("[ch2|t10x(7/5)]", h.debug());
}

TEST(Hit, Comparators)
{
  DAQuiri::Hit h, h1;
  DAQuiri::HitModel hm;
  hm.timebase = DAQuiri::TimeBase(2,1);
  DAQuiri::Hit h2(2, hm);
  ASSERT_TRUE(h == h1);
  ASSERT_FALSE(h1 == h2);
  ASSERT_TRUE(h1 != h2);
}


