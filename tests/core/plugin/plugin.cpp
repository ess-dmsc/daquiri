#include <gtest/gtest.h>
#include "plugin.h"

using namespace DAQuiri;

class Mock : public Plugin
{
  public:
    Mock() {}
    ~Mock() {}

    std::string plugin_name() const override { return "Mock"; }

    void settings(const Setting&) override {}
    Setting settings() const override { return Setting(); }
};

TEST(Plugin, Init)
{
  Mock m;
  EXPECT_EQ(m.plugin_name(), "Mock");
}
