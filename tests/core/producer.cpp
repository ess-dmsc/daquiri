#include <gtest/gtest.h>
#include "producer.h"

using namespace DAQuiri;

class Mock : public Producer
{
  public:
    Mock() {}
    ~Mock() {}

    std::string plugin_name() const override { return "Mock"; }

    void settings(const Setting&) override {}
    Setting settings() const override { return Setting(); }

    void boot() override {}
    void die() override {}
};

TEST(Producer, Init)
{
  Mock m;
  EXPECT_EQ(m.plugin_name(), "Mock");
}
