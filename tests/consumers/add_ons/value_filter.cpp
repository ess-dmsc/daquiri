#include <gtest/gtest.h>
#include <consumers/add_ons/value_filter.h>

using namespace DAQuiri;

TEST(ValueFilter, Init)
{
  ValueFilter h;
  EXPECT_FALSE(h.valid());
}

TEST(ValueFilter, Configure)
{
  Spill s;

  ValueFilter h;
  h.enabled_ = true;
  h.name_ = "val2";
  EXPECT_FALSE(h.valid());

  s.event_model.add_value("val", 100);
  h.configure(s);
  EXPECT_FALSE(h.valid());

  s.event_model.add_value("val2", 100);
  h.configure(s);
  EXPECT_TRUE(h.valid());
}

TEST(ValueFilter, ConfiguredFilters)
{
  Spill s;

  ValueFilter h;
  h.enabled_ = true;
  h.name_ = "val";
  h.min_ = 7;
  h.max_ = 42;

  s.event_model.add_value("val", 100);
  h.configure(s);
  EXPECT_TRUE(h.valid());

  Event e(s.event_model);

  e.set_value(0, 3);
  EXPECT_FALSE(h.accept(e));

  e.set_value(0, 7);
  EXPECT_TRUE(h.accept(e));

  e.set_value(0, 27);
  EXPECT_TRUE(h.accept(e));

  e.set_value(0, 42);
  EXPECT_TRUE(h.accept(e));

  e.set_value(0, 43);
  EXPECT_FALSE(h.accept(e));
}

TEST(ValueFilter, UnconfiguredAcceptsAll)
{
  Spill s;

  ValueFilter h;
  h.enabled_ = true;
  h.name_ = "val2";
  h.min_ = 7;
  h.max_ = 42;

  s.event_model.add_value("val", 100);
  h.configure(s);
  EXPECT_FALSE(h.valid());

  Event e(s.event_model);

  e.set_value(0, 3);
  EXPECT_TRUE(h.accept(e));

  e.set_value(0, 7);
  EXPECT_TRUE(h.accept(e));

  e.set_value(0, 27);
  EXPECT_TRUE(h.accept(e));

  e.set_value(0, 42);
  EXPECT_TRUE(h.accept(e));

  e.set_value(0, 43);
  EXPECT_TRUE(h.accept(e));
}

TEST(ValueFilter, GetSettings)
{
  ValueFilter h;

  h.enabled_ = true;
  h.name_ = "val";
  h.min_ = 7;
  h.max_ = 42;

  auto sets = h.settings(3);

  auto enabled = sets.find(Setting("filter/enabled"));
  EXPECT_TRUE(enabled.get_bool());
  EXPECT_TRUE(enabled.has_index(3));

  auto name = sets.find(Setting("filter/value_id"));
  EXPECT_EQ(name.get_text(), "val");
  EXPECT_TRUE(name.has_index(3));

  auto min = sets.find(Setting("filter/min"));
  EXPECT_EQ(min.get_number(), 7);
  EXPECT_TRUE(min.has_index(3));

  auto max = sets.find(Setting("filter/max"));
  EXPECT_EQ(max.get_number(), 42);
  EXPECT_TRUE(max.has_index(3));
}

TEST(ValueFilter, SetSettings)
{
  ValueFilter h;

  auto sets = h.settings(0);

  sets.set(Setting::boolean("filter/enabled", true));
  sets.set(Setting::text("filter/value_id", "val"));
  sets.set(Setting::integer("filter/min", 7));
  sets.set(Setting::integer("filter/max", 42));

  h.settings(sets);

  EXPECT_EQ(h.enabled_, true);
  EXPECT_EQ(h.name_, "val");
  EXPECT_EQ(h.min_, 7UL);
  EXPECT_EQ(h.max_, 42UL);
}