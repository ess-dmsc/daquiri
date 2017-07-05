#include "setting_metadata.h"
#include <gtest/gtest.h>

TEST(SettingMeta, Type)
{
  DAQuiri::SettingType t(DAQuiri::SettingType::none);
  ASSERT_EQ(DAQuiri::to_string(t), "");

  t = DAQuiri::from_string("stem");
  ASSERT_EQ(t, DAQuiri::SettingType::stem);
  ASSERT_EQ(DAQuiri::to_string(t), "stem");

  t = DAQuiri::from_string("boolean");
  ASSERT_EQ(t, DAQuiri::SettingType::boolean);
  ASSERT_EQ(DAQuiri::to_string(t), "boolean");

  t = DAQuiri::from_string("binary");
  ASSERT_EQ(t, DAQuiri::SettingType::binary);
  ASSERT_EQ(DAQuiri::to_string(t), "binary");

  t = DAQuiri::from_string("integer");
  ASSERT_EQ(t, DAQuiri::SettingType::integer);
  ASSERT_EQ(DAQuiri::to_string(t), "integer");

  t = DAQuiri::from_string("pattern");
  ASSERT_EQ(t, DAQuiri::SettingType::pattern);
  ASSERT_EQ(DAQuiri::to_string(t), "pattern");

  t = DAQuiri::from_string("floating");
  ASSERT_EQ(t, DAQuiri::SettingType::floating);
  ASSERT_EQ(DAQuiri::to_string(t), "floating");

  t = DAQuiri::from_string("precise");
  ASSERT_EQ(t, DAQuiri::SettingType::precise);
  ASSERT_EQ(DAQuiri::to_string(t), "precise");

  t = DAQuiri::from_string("text");
  ASSERT_EQ(t, DAQuiri::SettingType::text);
  ASSERT_EQ(DAQuiri::to_string(t), "text");

  t = DAQuiri::from_string("color");
  ASSERT_EQ(t, DAQuiri::SettingType::color);
  ASSERT_EQ(DAQuiri::to_string(t), "color");

  t = DAQuiri::from_string("time");
  ASSERT_EQ(t, DAQuiri::SettingType::time);
  ASSERT_EQ(DAQuiri::to_string(t), "time");

  t = DAQuiri::from_string("duration");
  ASSERT_EQ(t, DAQuiri::SettingType::duration);
  ASSERT_EQ(DAQuiri::to_string(t), "duration");

  t = DAQuiri::from_string("detector");
  ASSERT_EQ(t, DAQuiri::SettingType::detector);
  ASSERT_EQ(DAQuiri::to_string(t), "detector");

  t = DAQuiri::from_string("file");
  ASSERT_EQ(t, DAQuiri::SettingType::file);
  ASSERT_EQ(DAQuiri::to_string(t), "file");

  t = DAQuiri::from_string("dir");
  ASSERT_EQ(t, DAQuiri::SettingType::dir);
  ASSERT_EQ(DAQuiri::to_string(t), "dir");

  t = DAQuiri::from_string("command");
  ASSERT_EQ(t, DAQuiri::SettingType::command);
  ASSERT_EQ(DAQuiri::to_string(t), "command");

  t = DAQuiri::from_string("indicator");
  ASSERT_EQ(t, DAQuiri::SettingType::indicator);
  ASSERT_EQ(DAQuiri::to_string(t), "indicator");

  t = DAQuiri::from_string("menu");
  ASSERT_EQ(t, DAQuiri::SettingType::menu);
  ASSERT_EQ(DAQuiri::to_string(t), "menu");
}

TEST(SettingMeta, Init)
{
  DAQuiri::SettingMeta m;
  ASSERT_EQ(m.id(), "");
  ASSERT_EQ(m.type(), DAQuiri::SettingType::none);

  m = DAQuiri::SettingMeta("a", DAQuiri::SettingType::integer);
  ASSERT_EQ(m.id(), "a");
  ASSERT_EQ(m.type(), DAQuiri::SettingType::integer);
  ASSERT_EQ(m.get_string("name", ""), "a");

  m = DAQuiri::SettingMeta("a", DAQuiri::SettingType::integer, "b");
  ASSERT_EQ(m.id(), "a");
  ASSERT_EQ(m.type(), DAQuiri::SettingType::integer);
  ASSERT_EQ(m.get_string("name", ""), "b");
}

TEST(SettingMeta, Meaningful)
{
  DAQuiri::SettingMeta m("a", DAQuiri::SettingType::integer);
  ASSERT_TRUE(m.meaningful());

  m = m.stripped();
  ASSERT_FALSE(m.meaningful());
}

TEST(SettingMeta, Flags)
{
  DAQuiri::SettingMeta m;

  m.set_flag("a");
  ASSERT_TRUE(m.has_flag("a"));
  m.remove_flag("a");
  ASSERT_FALSE(m.has_flag("a"));

  ASSERT_FALSE(m.has_flag("b"));
  m.remove_flag("b");
  ASSERT_FALSE(m.has_flag("b"));

  m.set_flags({"c", "d"});
  ASSERT_TRUE(m.has_flag("c"));
  ASSERT_TRUE(m.has_flag("d"));
}

TEST(SettingMeta, Enum)
{
  DAQuiri::SettingMeta m;

  m.set_enum(1, "a");
  ASSERT_EQ(m.enum_name(1), "a");

  m.set_enum(2, "b");
  ASSERT_EQ(m.enum_names(), std::list<std::string>({"a", "b"}));
}

TEST(SettingMeta, Values)
{
  DAQuiri::SettingMeta m;

  ASSERT_EQ(m.get_string("a", "z"), "z");
  m.set_val("a", "aa");
  ASSERT_EQ(m.get_string("a", "z"), "aa");

  ASSERT_EQ(m.get_num("b", int(2)), 2);
  m.set_val("b", int(1));
  ASSERT_EQ(m.get_num("b", int(2)), 1);

  ASSERT_EQ(m.get_num("c", double(2.2)), 2.2);
  m.set_val("c", double(1.1));
  ASSERT_EQ(m.get_num("c", double(2.2)), 1.1);
}

TEST(SettingMeta, MinMaxStep)
{
  DAQuiri::SettingMeta m("", DAQuiri::SettingType::floating);

  ASSERT_EQ(m.min<int>(), std::numeric_limits<int>::min());
  m.set_val("min", int(-42));
  ASSERT_EQ(m.min<int>(), -42);

  ASSERT_EQ(m.max<int>(), std::numeric_limits<int>::max());
  m.set_val("max", int(42));
  ASSERT_EQ(m.max<int>(), 42);

  ASSERT_EQ(m.step<double>(), 1);
  m.set_val("step", double(0.5));
  ASSERT_EQ(m.step<double>(), 0.5);

  ASSERT_EQ(m.value_range(), "[-42 \uFF1A 0.5 \uFF1A 42]");
}

TEST(SettingMeta, Numeric)
{
  DAQuiri::SettingMeta m;
  ASSERT_FALSE(m.numeric());

  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::integer);
  ASSERT_TRUE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::floating);
  ASSERT_TRUE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::precise);
  ASSERT_TRUE(m.numeric());

  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::boolean);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::binary);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::pattern);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::text);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::color);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::detector);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::time);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::duration);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::file);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::dir);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::command);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::menu);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::indicator);
  ASSERT_FALSE(m.numeric());
  m = DAQuiri::SettingMeta("", DAQuiri::SettingType::stem);
  ASSERT_FALSE(m.numeric());
}

