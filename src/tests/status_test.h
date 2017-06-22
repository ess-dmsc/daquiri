#include "status.h"
#include <gtest/gtest.h>

TEST(Status, Init)
{
  DAQuiri::Status s;
  ASSERT_EQ(s.type(), DAQuiri::StatusType::running);
  ASSERT_EQ(s.channel(), -1);
}

TEST(Status, SetGet)
{
  DAQuiri::Status s;
  s.set_type(DAQuiri::StatusType::start);
  ASSERT_EQ(s.type(), DAQuiri::StatusType::start);

  s.set_channel(42);
  ASSERT_EQ(s.channel(), 42);

  auto t = boost::posix_time::microsec_clock::universal_time();
  s.set_time(t);
  ASSERT_EQ(s.time(), t);

  DAQuiri::HitModel hm;
  hm.timebase = DAQuiri::TimeBase(14,10);
  hm.add_value("energy", 16);
  hm.add_trace("wave", {3});

  s.set_model(hm);
  ASSERT_EQ(s.hit_model().timebase, hm.timebase);
  ASSERT_EQ(s.hit_model().value_names, hm.value_names);
  ASSERT_EQ(s.hit_model().trace_names, hm.trace_names);
}

