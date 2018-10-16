#include "gtest_color_print.h"
#include <consumers/tof_1d.h>

class TOF1D : public TestBase
{
  protected:
    virtual void SetUp()
    {
      h.set_attribute(DAQuiri::Setting::text("stream_id", "stream"));
      h.set_attribute(DAQuiri::Setting::floating("time_resolution", 1));
      h.set_attribute(DAQuiri::Setting::integer("time_units", 0));

      s.state.branches.add_a(DAQuiri::Setting::floating("pulse_time", 10));
      s.event_model.add_value("val", 100);
      s.events.reserve(3, s.event_model);
      s.events.last().set_time(10);
      s.events.last().set_value(0, 0);
      ++s.events;
      s.events.last().set_time(20);
      s.events.last().set_value(0, 15);
      ++s.events;
      s.events.last().set_time(20);
      s.events.last().set_value(0, 30);
      ++s.events;

      s.events.finalize();
    }

    DAQuiri::TOF1D h;
    DAQuiri::Spill s{"stream", DAQuiri::Spill::Type::start};
};

TEST_F(TOF1D, DefaultConstructed)
{
  DAQuiri::TOF1D default_h;
  EXPECT_FALSE(default_h.changed());
  EXPECT_EQ(default_h.type(), "Time of Flight 1D");
  EXPECT_EQ(default_h.dimensions(), 1);
}

TEST_F(TOF1D, HistogramsEvents)
{
  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 3);

  auto data = h.data()->range({});
  EXPECT_GE(data->size(), 10UL);
  EXPECT_EQ(data->begin()->first[0], 0UL);
  EXPECT_EQ(data->begin()->second, 1);
  EXPECT_EQ(data->rbegin()->first[0], 10UL);
  EXPECT_EQ(data->rbegin()->second, 2);
}

TEST_F(TOF1D, HistogramDecimate)
{
  h.set_attribute(DAQuiri::Setting::floating("time_resolution", 10));

  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 3);

  auto data = h.data()->range({});
  EXPECT_EQ(data->size(), 2UL);
  EXPECT_EQ(data->begin()->first[0], 0UL);
  EXPECT_EQ(data->begin()->second, 1);
  EXPECT_EQ(data->rbegin()->first[0], 1UL);
  EXPECT_EQ(data->rbegin()->second, 2);
}

TEST_F(TOF1D, LatchesStream)
{
  h.set_attribute(DAQuiri::Setting::text("stream_id", "N/A"));

  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 0);
}

TEST_F(TOF1D, ZeroResolutionBinsNothing)
{
  h.set_attribute(DAQuiri::Setting::floating("time_resolution", 0));
  h.push_spill(s);

  EXPECT_EQ(h.metadata().get_attribute("total_count").get_number(), 0);
}

TEST_F(TOF1D, FilterByValue)
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

//TODO: test other time units
//TODO: test data axes

TEST_F(TOF1D, Clone)
{
  h.push_spill(s);

  auto h_copy = std::shared_ptr<DAQuiri::TOF1D>(h.clone());

  EXPECT_NE(h_copy.get(), &h);
  EXPECT_EQ(h_copy->metadata().get_attribute("total_count").get_number(), 3);
  auto data = h_copy->data()->range({});
  EXPECT_GE(data->size(), 10UL);
  EXPECT_EQ(data->begin()->first[0], 0UL);
  EXPECT_EQ(data->begin()->second, 1);
  EXPECT_EQ(data->rbegin()->first[0], 10UL);
  EXPECT_EQ(data->rbegin()->second, 2);
}
