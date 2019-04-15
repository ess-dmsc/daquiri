#include "gtest_color_print.h"
#include <consumers/tof_val_2d_correlate.h>

class TOFVal2DCorrelate : public TestBase
{
  protected:
    virtual void SetUp()
    {
      h.set_attribute(DAQuiri::Setting::text("stream_id", "stream"));
      h.set_attribute(DAQuiri::Setting::text("value_latch/value_id", "val"));
      h.set_attribute(DAQuiri::Setting::floating("time_resolution", 1));
      h.set_attribute(DAQuiri::Setting::integer("time_units", 0));
      h.set_attribute(DAQuiri::Setting::text("chopper_stream_id", "chopper_stream"));

      s.event_model.add_value("val", 100);
      s.event_model.add_value("val2", 100);
      s.events.reserve(5, s.event_model);
      s.events.last().set_time(0);
      s.events.last().set_value(0, 0);
      s.events.last().set_value(1, 0);
      ++s.events;
      s.events.last().set_time(5);
      s.events.last().set_value(0, 15);
      s.events.last().set_value(1, 1);
      ++s.events;
      s.events.last().set_time(10);
      s.events.last().set_value(0, 15);
      s.events.last().set_value(1, 15);
      ++s.events;
      s.events.last().set_time(15);
      s.events.last().set_value(0, 30);
      s.events.last().set_value(1, 30);
      ++s.events;
      s.events.last().set_time(50);
      s.events.last().set_value(0, 30);
      s.events.last().set_value(1, 30);
      ++s.events;
      s.events.finalize();

      cs.events.reserve(4, cs.event_model);
      cs.events.last().set_time(0);
      ++cs.events;
      cs.events.last().set_time(10);
      ++cs.events;
      cs.events.last().set_time(20);
      ++cs.events;
      cs.events.last().set_time(30);
      ++cs.events;
      cs.events.finalize();
    }

    DAQuiri::TOFVal2DCorrelate h;
    DAQuiri::Spill s{"stream", DAQuiri::Spill::Type::start};
    DAQuiri::Spill cs{"chopper_stream", DAQuiri::Spill::Type::start};
};

TEST_F(TOFVal2DCorrelate, DefaultConstructed)
{
  DAQuiri::TOFVal2DCorrelate default_h;
  EXPECT_FALSE(default_h.changed());
  EXPECT_EQ(default_h.type(),
      "Time of Flight 2D (w/ stream correlation)");
  EXPECT_EQ(default_h.dimensions(), 2);
}

//TODO: not entirely clear how this should work...

TEST_F(TOFVal2DCorrelate, HistogramsEvents)
{
  h.push_spill(cs);
  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 0);

  h.push_spill(s);
  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 2);

  auto data = h.data()->range({});
  EXPECT_GE(data->size(), 2UL);
  EXPECT_EQ(data->begin()->first[0], 0UL);
  EXPECT_EQ(data->begin()->second, 1);
  EXPECT_EQ(data->rbegin()->first[0], 5UL);
  EXPECT_EQ(data->rbegin()->second, 1);
}


TEST_F(TOFVal2DCorrelate, HistogramDecimate)
{
  h.set_attribute(DAQuiri::Setting::floating("time_resolution", 5));

  h.push_spill(cs);
  h.push_spill(s);
  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 2);

  auto data = h.data()->range({});
  EXPECT_GE(data->size(), 2UL);
  EXPECT_EQ(data->begin()->first[0], 0UL);
  EXPECT_EQ(data->begin()->second, 1);
  EXPECT_EQ(data->rbegin()->first[0], 1UL);
  EXPECT_EQ(data->rbegin()->second, 1);

}

TEST_F(TOFVal2DCorrelate, LatchesStream)
{
  h.set_attribute(DAQuiri::Setting::text("stream_id", "N/A"));

  h.push_spill(cs);
  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 0);
}

TEST_F(TOFVal2DCorrelate, LatchesChopperStream)
{
  h.set_attribute(DAQuiri::Setting::text("chopper_stream_id", "N/A"));

  h.push_spill(cs);
  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 0);
}

TEST_F(TOFVal2DCorrelate, ZeroResolutionBinsNothing)
{
  h.set_attribute(DAQuiri::Setting::floating("time_resolution", 0));

  h.push_spill(cs);
  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 0);
}

/*
TEST_F(TOFVal2DCorrelate, FilterByValue)
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

  h.push_spill(cs);
  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 1);
}
*/

//TODO: test other time units
//TODO: test data axes

TEST_F(TOFVal2DCorrelate, Clone)
{
  h.push_spill(cs);
  h.push_spill(s);

  auto h_copy = std::shared_ptr<DAQuiri::TOFVal2DCorrelate>(h.clone());

  EXPECT_NE(h_copy.get(), &h);
  EXPECT_EQ(h_copy->metadata().get_attribute("total_count").get_number(), 2);
  auto data = h_copy->data()->range({});
  EXPECT_GE(data->size(), 2UL);
  EXPECT_EQ(data->begin()->first[0], 0UL);
  EXPECT_EQ(data->begin()->second, 1);
  EXPECT_EQ(data->rbegin()->first[0], 5UL);
  EXPECT_EQ(data->rbegin()->second, 1);
}
