#include <gtest/gtest.h>
#include <consumers/add_ons/filter_block.h>

using namespace DAQuiri;

TEST(FilterBlock, Init)
{
  FilterBlock h;
  EXPECT_FALSE(h.valid);
}

TEST(FilterBlock, GetSettings)
{
  FilterBlock h;
  auto sets = h.settings();
  EXPECT_EQ(sets.find(Setting("filter_count")).get_number(), 0);
}

TEST(FilterBlock, ResizeFiltersUp)
{
  FilterBlock h;
  auto sets = h.settings();
  sets.set(Setting::integer("filter_count", 3));
  h.settings(sets);
  EXPECT_EQ(h.filters_.size(), 3UL);
}

TEST(FilterBlock, ResizeFiltersDown)
{
  FilterBlock h;
  h.filters_.resize(3);
  auto sets = h.settings();
  sets.set(Setting::integer("filter_count", 2));
  h.settings(sets);
  EXPECT_EQ(h.filters_.size(), 2UL);
}

TEST(FilterBlock, ConfigureValid)
{
  FilterBlock h;
  h.filters_.resize(1);
  h.filters_[0].enabled_ = true;
  h.filters_[0].name_ = "val";

  Spill s;
  s.event_model.add_value("val", 100);

  h.configure(s);
  EXPECT_TRUE(h.valid);
}

TEST(FilterBlock, ConfigureInvalid)
{
  FilterBlock h;
  h.filters_.resize(1);
  h.filters_[0].enabled_ = true;
  h.filters_[0].name_ = "val2";

  Spill s;
  s.event_model.add_value("val", 100);

  h.configure(s);
  EXPECT_FALSE(h.valid);
}

TEST(FilterBlock, AcceptValid)
{
  FilterBlock h;
  h.filters_.resize(1);
  h.filters_[0].enabled_ = true;
  h.filters_[0].name_ = "val";
  h.filters_[0].min_ = 7;
  h.filters_[0].max_ = 42;

  Spill s;
  s.event_model.add_value("val", 100);
  h.configure(s);

  Event e(s.event_model);

  e.set_value(0, 6);
  EXPECT_FALSE(h.accept(e));

  e.set_value(0, 7);
  EXPECT_TRUE(h.accept(e));

  e.set_value(0, 42);
  EXPECT_TRUE(h.accept(e));

  e.set_value(0, 43);
  EXPECT_FALSE(h.accept(e));
}

TEST(FilterBlock, InvalidAcceptsAll)
{
  FilterBlock h;
  h.filters_.resize(1);
  h.filters_[0].enabled_ = true;
  h.filters_[0].name_ = "val2";
  h.filters_[0].min_ = 7;
  h.filters_[0].max_ = 42;

  Spill s;
  s.event_model.add_value("val", 100);
  h.configure(s);

  Event e(s.event_model);

  e.set_value(0, 6);
  EXPECT_TRUE(h.accept(e));

  e.set_value(0, 7);
  EXPECT_TRUE(h.accept(e));

  e.set_value(0, 42);
  EXPECT_TRUE(h.accept(e));

  e.set_value(0, 43);
  EXPECT_TRUE(h.accept(e));
}
