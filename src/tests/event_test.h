#include "event.h"
#include <gtest/gtest.h>

TEST(Event, Init)
{
  DAQuiri::Event h;
  ASSERT_EQ(-1, h.channel());
  ASSERT_EQ(DAQuiri::TimeStamp(), h.timestamp());
  ASSERT_EQ(0, h.value_count());
  ASSERT_EQ(0, h.trace_count());
  EXPECT_EQ("[ch-1|t0]", h.debug());

  DAQuiri::EventModel hm;
  hm.timebase = DAQuiri::TimeBase(14,10);
  hm.add_value("energy", 16);
  hm.add_trace("wave", {3});
  DAQuiri::Event h2(2, hm);
  ASSERT_EQ(2, h2.channel());
  ASSERT_EQ(1, h2.value_count());
  ASSERT_EQ(1, h2.trace_count());
  EXPECT_EQ("[ch2|t0x(7/5)|ntraces=1 0]", h2.debug());
}

TEST(Event, Value)
{
  DAQuiri::EventModel hm;
  hm.timebase = DAQuiri::TimeBase(14,10);
  hm.add_value("energy", 16);
  DAQuiri::Event h(2, hm);
//  ASSERT_ANY_THROW(h.value(2));
  ASSERT_NO_THROW(h.set_value(0,42));
  ASSERT_EQ(42, h.value(0));
  EXPECT_EQ("[ch2|t0x(7/5) 42]", h.debug());
}

TEST(Event, Trace)
{
  DAQuiri::EventModel hm;
  hm.timebase = DAQuiri::TimeBase(14,10);
  hm.add_trace("wave", {3});
  DAQuiri::Event h(2, hm);
  ASSERT_ANY_THROW(h.trace(2));
  ASSERT_NO_THROW(h.set_trace(0, {3,6,9}));
  ASSERT_EQ(std::vector<uint32_t>({3,6,9}),
            h.trace(0));
  EXPECT_EQ("[ch2|t0x(7/5)|ntraces=1]", h.debug());
}

TEST(Event, Time)
{
  DAQuiri::EventModel hm;
  hm.timebase = DAQuiri::TimeBase(2,1);
  DAQuiri::Event h(2, hm);
  h.set_time(10);
  ASSERT_EQ(10, h.timestamp());
  EXPECT_EQ("[ch2|t10]", h.debug());
}

TEST(Event, Comparators)
{
  DAQuiri::Event h, h1;
  DAQuiri::EventModel hm;
  hm.timebase = DAQuiri::TimeBase(2,1);
  DAQuiri::Event h2(2, hm);
  ASSERT_TRUE(h == h1);
  ASSERT_FALSE(h1 == h2);
  ASSERT_TRUE(h1 != h2);
}


