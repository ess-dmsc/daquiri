#include <gtest/gtest.h>
#include "consumer.h"

using namespace DAQuiri;

class MockConsumer : public Consumer
{
  public:
    MockConsumer() {}
    MockConsumer* clone() const override { return new MockConsumer(*this); }

  protected:
    std::string my_type() const override { return "MockConsumer"; }
    void _recalc_axes() override {}
    bool _accept_spill(const Spill& spill) override { return true; }
    bool _accept_events(const Spill& spill) override { return true; }
    void _push_event(const Event&) override {}
};

TEST(Consumer, Init)
{
  MockConsumer c;
  EXPECT_EQ(c.type(), "MockConsumer");
}