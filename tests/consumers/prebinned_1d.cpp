#include "gtest_color_print.h"
#include <consumers/prebinned_1d.h>

class Prebinned1D : public TestBase
{
  protected:
    virtual void SetUp()
    {
      h.set_attribute(DAQuiri::Setting::text("stream_id", "stream"));
      h.set_attribute(DAQuiri::Setting::text("trace_id", "trace"));

      s.event_model.add_value("val", 100);
      s.event_model.add_trace("trace", {3});
      s.events.reserve(3, s.event_model);
      s.events.last().trace(0)[0] = 1;
      s.events.last().trace(0)[1] = 0;
      s.events.last().trace(0)[2] = 1;
      s.events.last().set_value(0, 0);
      ++s.events;
      s.events.last().trace(0)[0] = 0;
      s.events.last().trace(0)[1] = 1;
      s.events.last().trace(0)[2] = 0;
      s.events.last().set_value(0, 15);
      ++s.events;
      s.events.last().trace(0)[0] = 1;
      s.events.last().trace(0)[1] = 0;
      s.events.last().trace(0)[2] = 0;
      s.events.last().set_value(0, 30);
      ++s.events;

      s.events.finalize();
    }

    DAQuiri::Prebinned1D h;
    DAQuiri::Spill s{"stream", DAQuiri::Spill::Type::start};
};

TEST_F(Prebinned1D, DefaultConstructed)
{
  DAQuiri::Prebinned1D default_h;
  EXPECT_FALSE(default_h.changed());
  EXPECT_EQ(default_h.type(), "Prebinned 1D");
  EXPECT_EQ(default_h.dimensions(), 1);
}

TEST_F(Prebinned1D, HistogramsEvents)
{
  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 4);

  auto data = h.data()->range({});
  ASSERT_EQ(data->size(), 3UL);
  EXPECT_EQ(data->at(0).first[0], 0UL);
  EXPECT_EQ(data->at(0).second, 2);
  EXPECT_EQ(data->at(1).first[0], 1UL);
  EXPECT_EQ(data->at(1).second, 1);
  EXPECT_EQ(data->at(2).first[0], 2UL);
  EXPECT_EQ(data->at(2).second, 1);
}

TEST_F(Prebinned1D, LatchesTrace)
{
  h.set_attribute(DAQuiri::Setting::text("trace_id", "N/A"));

  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 0);
}

TEST_F(Prebinned1D, LatchesStream)
{
  h.set_attribute(DAQuiri::Setting::text("stream_id", "N/A"));

  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 0);
}

TEST_F(Prebinned1D, FilterByValue)
{
  h.set_attribute(DAQuiri::Setting::integer("filter_count", 1));

  auto filters = h.metadata().get_attribute("filters");

  auto fe = DAQuiri::Setting::boolean("filter/enabled", true);
  fe.set_indices({0});
  h.set_attribute(fe);

  auto fn = DAQuiri::Setting::text("filter/value_id", "val");
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

TEST_F(Prebinned1D, Clone)
{
  h.push_spill(s);

  auto h_copy = std::shared_ptr<DAQuiri::Prebinned1D>(h.clone());

  EXPECT_NE(h_copy.get(), &h);
  EXPECT_EQ(h_copy->metadata().get_attribute("total_count").get_number(), 4);
  auto data = h_copy->data()->range({});
  ASSERT_EQ(data->size(), 3UL);
  EXPECT_EQ(data->at(0).first[0], 0UL);
  EXPECT_EQ(data->at(0).second, 2);
  EXPECT_EQ(data->at(1).first[0], 1UL);
  EXPECT_EQ(data->at(1).second, 1);
  EXPECT_EQ(data->at(2).first[0], 2UL);
  EXPECT_EQ(data->at(2).second, 1);
}
