#include <gtest/gtest.h>
#include <core/consumer.h>

using namespace DAQuiri;

class MockConsumer : public Consumer
{
  public:
    MockConsumer() {}
    MockConsumer* clone() const override { return new MockConsumer(*this); }

    bool accept_events{false};

    size_t accepted_spills{0};
    size_t accepted_events{0};

  protected:
    std::string my_type() const override { return "MockConsumer"; }
    void _recalc_axes() override {}
    bool _accept_spill(const Spill& spill) override
    {
      if (!Consumer::_accept_spill(spill))
        return false;
      accepted_spills++;
      return true;
    }
    bool _accept_events(const Spill&) override { return accept_events; }
    void _push_event(const Event&) override { accepted_events++; }
};

TEST(Consumer, DefaultConstructor)
{
  MockConsumer c;
  EXPECT_EQ(c.type(), "MockConsumer");
  EXPECT_EQ(c.dimensions(), 0);
  EXPECT_FALSE(c.data());
  EXPECT_FALSE(c.changed());
}

TEST(Consumer, GetMetadata)
{
  MockConsumer c;

  auto md = c.metadata();
  EXPECT_TRUE(md.get_attribute("stream_id"));
}

TEST(Consumer, AcceptingSpillsByID)
{
  Spill s("", Spill::Type::daq_status);

  MockConsumer c;
  c.push_spill(s);
  c.push_spill(s);
  EXPECT_EQ(c.accepted_spills, 2UL);

  Spill s2("someid", Spill::Type::daq_status);
  c.push_spill(s2);
  c.push_spill(s2);
  EXPECT_EQ(c.accepted_spills, 2UL);

  c.set_attribute(Setting::text("stream_id", "someid"));
  c.push_spill(s2);
  c.push_spill(s2);
  EXPECT_EQ(c.accepted_spills, 4UL);
}

TEST(Consumer, EventsArePushedWhenAccepted)
{
  Spill s("", Spill::Type::daq_status);
  s.events.reserve(3, Event(EventModel()));
  ++s.events;
  ++s.events;
  ++s.events;
  s.events.finalize();

  MockConsumer c;
  c.push_spill(s);
  EXPECT_EQ(c.accepted_events, 0UL);

  c.accept_events = true;
  c.push_spill(s);
  EXPECT_EQ(c.accepted_events, 3UL);
}

//TODO: this is failing
//TEST(Consumer, ChangeAndReset)
//{
//  Spill s("", Spill::Type::daq_status);
//
//  MockConsumer c;
//  c.push_spill(s);
//  EXPECT_TRUE(c.changed());
//
//  c.reset_changed();
//  EXPECT_FALSE(c.changed());
//}
