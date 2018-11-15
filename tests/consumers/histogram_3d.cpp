#include "gtest_color_print.h"
#include <consumers/histogram_3d.h>

class Histogram3D : public TestBase
{
  protected:
    virtual void SetUp()
    {
      h.set_attribute(DAQuiri::Setting::text("stream_id", "stream"));

      auto vx = h.metadata().get_attribute("value_latch/value_id", 0);
      vx.set_text("x");
      h.set_attribute(vx);

      auto vy = h.metadata().get_attribute("value_latch/value_id", 1);
      vy.set_text("y");
      h.set_attribute(vy);

      auto vz = h.metadata().get_attribute("value_latch/value_id", 2);
      vz.set_text("z");
      h.set_attribute(vz);

      s.event_model.add_value("x", 100);
      s.event_model.add_value("y", 100);
      s.event_model.add_value("z", 100);
      s.event_model.add_value("filter_val", 100);
      s.events.reserve(3, s.event_model);

      s.events.last().set_value(0, 0);
      s.events.last().set_value(1, 0);
      s.events.last().set_value(2, 0);
      s.events.last().set_value(3, 0);
      ++s.events;
      s.events.last().set_value(0, 2);
      s.events.last().set_value(1, 2);
      s.events.last().set_value(2, 2);
      s.events.last().set_value(3, 15);
      ++s.events;
      s.events.last().set_value(0, 2);
      s.events.last().set_value(1, 2);
      s.events.last().set_value(2, 2);
      s.events.last().set_value(3, 30);
      ++s.events;
      s.events.finalize();
    }

    DAQuiri::Histogram3D h;
    DAQuiri::Spill s{"stream", DAQuiri::Spill::Type::start};
};

TEST_F(Histogram3D, DefaultConstructed)
{
  DAQuiri::Histogram3D default_h;
  EXPECT_FALSE(default_h.changed());
  EXPECT_EQ(default_h.type(), "Histogram 3D");
  EXPECT_EQ(default_h.dimensions(), 3);
}

TEST_F(Histogram3D, HistogramsEvents)
{
  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 3);

  auto data = h.data()->range({});
  ASSERT_GE(data->size(), 2UL);
  EXPECT_EQ(data->begin()->first[0], 0UL);
  EXPECT_EQ(data->begin()->first[1], 0UL);
  EXPECT_EQ(data->begin()->first[2], 0UL);
  EXPECT_EQ(data->begin()->second, 1);
  EXPECT_EQ(data->rbegin()->first[0], 2UL);
  EXPECT_EQ(data->rbegin()->first[1], 2UL);
  EXPECT_EQ(data->rbegin()->first[2], 2UL);
  EXPECT_EQ(data->rbegin()->second, 2);
}

TEST_F(Histogram3D, LatchesValue)
{
  auto vx = h.metadata().get_attribute("value_latch/value_id", 0);
  vx.set_text("N/A");
  h.set_attribute(vx);

  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 0);
}

TEST_F(Histogram3D, LatchesStream)
{
  h.set_attribute(DAQuiri::Setting::text("stream_id", "N/A"));

  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 0);
}

TEST_F(Histogram3D, FilterByValue)
{
  h.set_attribute(DAQuiri::Setting::integer("filter_count", 1));

  auto filters = h.metadata().get_attribute("filters");

  auto fe = DAQuiri::Setting::boolean("filter/enabled", true);
  fe.set_indices({0});
  h.set_attribute(fe);

  auto fn = DAQuiri::Setting::text("filter/value_id", "filter_val");
  fn.set_indices({0});
  h.set_attribute(fn);

  auto fmin = DAQuiri::Setting::integer("filter/min", 10);
  fmin.set_indices({0});
  h.set_attribute(fmin);

  auto fmax= DAQuiri::Setting::integer("filter/max", 20);
  fmax.set_indices({0});
  h.set_attribute(fmax);

  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 1);
}

TEST_F(Histogram3D, Clone)
{
  h.push_spill(s);

  auto h_copy = std::shared_ptr<DAQuiri::Histogram3D>(h.clone());

  EXPECT_NE(h_copy.get(), &h);
  EXPECT_EQ(h_copy->metadata().get_attribute("total_count").get_number(), 3);
  auto data = h_copy->data()->range({});
  ASSERT_GE(data->size(), 2UL);
  EXPECT_EQ(data->begin()->first[0], 0UL);
  EXPECT_EQ(data->begin()->first[1], 0UL);
  EXPECT_EQ(data->begin()->first[2], 0UL);
  EXPECT_EQ(data->begin()->second, 1);
  EXPECT_EQ(data->rbegin()->first[0], 2UL);
  EXPECT_EQ(data->rbegin()->first[1], 2UL);
  EXPECT_EQ(data->rbegin()->first[2], 2UL);
  EXPECT_EQ(data->rbegin()->second, 2);
}
