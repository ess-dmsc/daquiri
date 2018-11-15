#include "gtest_color_print.h"
#include <core/spill.h>

class EventBuffer : public TestBase
{
 protected:
  virtual void SetUp()
  {
    DAQuiri::EventModel hm;
    hm.timebase = DAQuiri::TimeBase(14,10);
    hm.add_value("energy", 16);
    e = DAQuiri::Event(hm);
  }

  DAQuiri::Event e;
};

TEST_F(EventBuffer, Init)
{
  DAQuiri::EventBuffer eb;
  EXPECT_TRUE(eb.empty());
  EXPECT_EQ(eb.size(), 0UL);
}

TEST_F(EventBuffer, reserve)
{
  DAQuiri::EventBuffer eb;
  eb.reserve(10, e);
  EXPECT_FALSE(eb.empty());
  EXPECT_EQ(eb.size(), 10UL);
  EXPECT_EQ(eb.last().value_count(), 1UL);
}

TEST_F(EventBuffer, finalize)
{
  DAQuiri::EventBuffer eb;
  eb.reserve(10, e);
  EXPECT_FALSE(eb.empty());
  EXPECT_EQ(eb.size(), 10UL);

  eb.finalize();
  EXPECT_TRUE(eb.empty());
  EXPECT_EQ(eb.size(), 0UL);

  eb++;
  ++eb;
  eb.finalize();
  EXPECT_FALSE(eb.empty());
  EXPECT_EQ(eb.size(), 2UL);
}


TEST_F(EventBuffer, iterate)
{
  DAQuiri::EventBuffer eb;
  eb.reserve(10, e);

  for (size_t i=0; i <10; ++i)
  {
    eb.last().set_value(0,i);
    eb++;
  }

  eb.finalize();
  EXPECT_EQ(eb.size(), 10UL);
  uint16_t x {0};
  for (const auto& e : eb)
  {
    EXPECT_EQ(e.value(0), x);
    x++;
  }
}

class Spill : public TestBase
{
 protected:
  virtual void SetUp()
  {
    model.timebase = DAQuiri::TimeBase(14,10);
    model.add_value("energy", 16);
    e = DAQuiri::Event(model);
  }

  DAQuiri::EventModel model;
  DAQuiri::Event e;
};

TEST_F(Spill, Init)
{
  DAQuiri::Spill s;
  EXPECT_TRUE(s.stream_id.empty());
  EXPECT_EQ(s.type, DAQuiri::Spill::Type::daq_status);
  EXPECT_TRUE(s.raw.empty());
  EXPECT_TRUE(s.events.empty());
  EXPECT_FALSE(s.state);
  EXPECT_TRUE(s.empty());
}

TEST_F(Spill, NonemptyIfStateDefined)
{
  DAQuiri::Spill s;
  s.state = DAQuiri::Setting::stem("some_id");
  EXPECT_FALSE(s.empty());
}

TEST_F(Spill, NonemptyIfRawData)
{
  DAQuiri::Spill s;
  s.raw.resize(10);
  EXPECT_FALSE(s.empty());
}

TEST_F(Spill, NonemptyIfEventData)
{
  DAQuiri::Spill s;
  s.events.reserve(10, e);
  EXPECT_FALSE(s.empty());
}

TEST_F(Spill, InitWithType)
{
  DAQuiri::Spill s("stream_id_x", DAQuiri::Spill::Type::start);
  EXPECT_EQ(s.stream_id, "stream_id_x");
  EXPECT_EQ(s.type, DAQuiri::Spill::Type::start);
  EXPECT_TRUE(s.raw.empty());
  EXPECT_TRUE(s.events.empty());
  EXPECT_TRUE(s.state);
  EXPECT_FALSE(s.empty());
}

TEST_F(Spill, TypeToStr)
{
  EXPECT_EQ(DAQuiri::Spill::to_str(DAQuiri::Spill::Type::start), "start");
  EXPECT_EQ(DAQuiri::Spill::to_str(DAQuiri::Spill::Type::stop), "stop");
  EXPECT_EQ(DAQuiri::Spill::to_str(DAQuiri::Spill::Type::daq_status), "daq_status");
  EXPECT_EQ(DAQuiri::Spill::to_str(DAQuiri::Spill::Type::running), "running");
}

TEST_F(Spill, TypeFromStr)
{
  EXPECT_EQ(DAQuiri::Spill::from_str("start"), DAQuiri::Spill::Type::start);
  EXPECT_EQ(DAQuiri::Spill::from_str("stop"), DAQuiri::Spill::Type::stop);
  EXPECT_EQ(DAQuiri::Spill::from_str("daq_status"), DAQuiri::Spill::Type::daq_status);
  EXPECT_EQ(DAQuiri::Spill::from_str("running"), DAQuiri::Spill::Type::running);
}

TEST_F(Spill, ToJson)
{
  DAQuiri::Spill s("stream_id_x", DAQuiri::Spill::Type::start);
  s.event_model = model;
  s.events.reserve(10, e);
  s.raw.resize(25);

  json j = s;
  EXPECT_EQ(j["type"], "start");
  EXPECT_EQ(j["stream_id"], "stream_id_x");
  EXPECT_EQ(j["time"], to_iso_extended(s.time));
  EXPECT_EQ(j["event_model"]["values"].size(), 1UL);
  EXPECT_EQ(j["state"]["type"], "stem");
}

TEST_F(Spill, DebugString)
{
  DAQuiri::Spill s("stream_id_x", DAQuiri::Spill::Type::start);
  s.event_model = model;
  s.events.reserve(10, e);
  s.raw.resize(25);

  EXPECT_FALSE(s.debug("").empty());
}