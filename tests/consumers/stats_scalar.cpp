#include "gtest_color_print.h"
#include <consumers/stats_scalar.h>

class StatsScalar : public TestBase
{
  protected:
    virtual void SetUp()
    {
      h.set_attribute(DAQuiri::Setting::text("stream_id", "stream"));
      h.set_attribute(DAQuiri::Setting::text("what_stats", "stat1"));
      h.set_attribute(DAQuiri::Setting::boolean("diff", false));
    }

    DAQuiri::StatsScalar h;
    DAQuiri::Spill s{"stream", DAQuiri::Spill::Type::start};
};

TEST_F(StatsScalar, DefaultConstructed)
{
  DAQuiri::StatsScalar default_h;
  EXPECT_FALSE(default_h.changed());
  EXPECT_EQ(default_h.type(), "Stats Scalar");
  EXPECT_EQ(default_h.dimensions(), 0);
}

TEST_F(StatsScalar, StatsNoDelta)
{
  s.state.branches.add_a(DAQuiri::Setting::floating("stat1", 0));
  h.push_spill(s);
  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 1);

  s.state.set(DAQuiri::Setting::floating("stat1", 10));
  h.push_spill(s);
  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 2);

  s.state.set(DAQuiri::Setting::floating("stat1", 5));
  h.push_spill(s);
  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 3);

  EXPECT_EQ(h.data()->get({}), 5);

  auto data = h.data()->range({});

  EXPECT_EQ(data->size(), 2UL);
  EXPECT_EQ(data->begin()->second, 0);
  EXPECT_EQ(data->rbegin()->second, 10);
}

TEST_F(StatsScalar, StatsDelta)
{
  h.set_attribute(DAQuiri::Setting::boolean("diff", true));

  s.state.branches.add_a(DAQuiri::Setting::floating("stat1", 0));
  h.push_spill(s);
  EXPECT_EQ(h.data()->get({}), 0);
  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 1);

  s.state.set(DAQuiri::Setting::floating("stat1", 10));
  h.push_spill(s);
  EXPECT_EQ(h.data()->get({}), 10);
  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 2);

  s.state.set(DAQuiri::Setting::floating("stat1", 5));
  h.push_spill(s);
  EXPECT_EQ(h.data()->get({}), -5);
  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 3);

  h.flush();
  EXPECT_EQ(h.data()->get({}), 0);
  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 4);

  auto data = h.data()->range({});

  EXPECT_EQ(data->size(), 2UL);
  EXPECT_EQ(data->begin()->second, -5);
  EXPECT_EQ(data->rbegin()->second, 10);
}

TEST_F(StatsScalar, LatchesStream)
{
  h.set_attribute(DAQuiri::Setting::text("stream_id", "N/A"));

  s.state.branches.add_a(DAQuiri::Setting::floating("stat1", 0));
  h.push_spill(s);
  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 0);
}

TEST_F(StatsScalar, LatchesStat)
{
  h.set_attribute(DAQuiri::Setting::text("stream_id", "N/A"));

  s.state.branches.add_a(DAQuiri::Setting::floating("stat2", 0));
  h.push_spill(s);
  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 0);
}

TEST_F(StatsScalar, Clone)
{
  s.state.branches.add_a(DAQuiri::Setting::floating("stat1", 0));
  h.push_spill(s);

  s.state.set(DAQuiri::Setting::floating("stat1", 10));
  h.push_spill(s);

  s.state.set(DAQuiri::Setting::floating("stat1", 5));
  h.push_spill(s);

  auto h_copy = std::shared_ptr<DAQuiri::StatsScalar>(h.clone());

  EXPECT_NE(h_copy.get(), &h);
  EXPECT_EQ(h_copy->metadata().get_attribute("total_count").get_number(), 3);

  EXPECT_EQ(h_copy->data()->get({}), 5);

  auto data = h_copy->data()->range({});

  EXPECT_EQ(data->size(), 2UL);
  EXPECT_EQ(data->begin()->second, 0);
  EXPECT_EQ(data->rbegin()->second, 10);
}
