#include "gtest_color_print.h"
#include <consumers/add_ons/value_latch.h>

class ValueLatch : public TestBase
{
  protected:
    virtual void SetUp()
    {
      vl.value_id = "val";
    }

    DAQuiri::ValueLatch vl;
    DAQuiri::Spill s;
};

TEST_F(ValueLatch, Init)
{
  EXPECT_FALSE(vl.valid());
}

TEST_F(ValueLatch, Configure)
{
  EXPECT_FALSE(vl.valid());

  s.event_model.add_value("val2", 100);
  vl.configure(s);
  EXPECT_FALSE(vl.valid());

  s.event_model.add_value("val", 100);
  vl.configure(s);
  EXPECT_TRUE(vl.valid());
}

TEST_F(ValueLatch, ExtractNoDownsample)
{
  s.event_model.add_value("val", 100);
  vl.configure(s);

  DAQuiri::Event e(s.event_model);
  size_t result;

  e.set_value(0, 3);
  vl.extract(result, e);
  EXPECT_EQ(result, 3UL);
}

TEST_F(ValueLatch, ExtractWithDownsample)
{
  s.event_model.add_value("val", 100);
  vl.configure(s);
  vl.downsample = 2;

  DAQuiri::Event e(s.event_model);
  size_t result;

  e.set_value(0, 16);
  vl.extract(result, e);
  EXPECT_EQ(result, 4UL);
}

TEST_F(ValueLatch, GetSettings)
{
  vl.value_id = "val";
  vl.downsample = 4;

  auto sets = vl.settings(3);
  EXPECT_EQ(sets.id(), "value_latch");

  auto name = sets.find(DAQuiri::Setting("value_latch/value_id"));
  EXPECT_EQ(name.get_text(), "val");
  EXPECT_TRUE(name.has_index(3));

  auto ds = sets.find(DAQuiri::Setting("value_latch/downsample"));
  EXPECT_EQ(ds.get_int(), 4);
  EXPECT_TRUE(ds.has_index(3));
}

TEST_F(ValueLatch, GetSettingsOverrideName)
{
  vl.value_id = "val";
  vl.downsample = 4;

  auto sets = vl.settings(0, "NewName");
  EXPECT_EQ(sets.metadata().get_string("name", ""), "NewName");
}


TEST_F(ValueLatch, SetSettings)
{
  auto sets = vl.settings();

  sets.set(DAQuiri::Setting::text("value_latch/value_id", "val2"));
  sets.set(DAQuiri::Setting::integer("value_latch/downsample", 7));

  vl.settings(sets);

  EXPECT_EQ(vl.value_id, "val2");
  EXPECT_EQ(vl.downsample, 7);
}
