#include <core/plugin/setting_metadata.h>
#include "gtest_color_print.h"

class SettingMeta : public TestBase
{
};

TEST_F(SettingMeta, TypeString)
{
  DAQuiri::SettingType t;

  t = DAQuiri::from_string("");
  EXPECT_EQ(t, DAQuiri::SettingType::none);
  EXPECT_EQ(DAQuiri::to_string(t), "");

  t = DAQuiri::from_string("stem");
  EXPECT_EQ(t, DAQuiri::SettingType::stem);
  EXPECT_EQ(DAQuiri::to_string(t), "stem");

  t = DAQuiri::from_string("boolean");
  EXPECT_EQ(t, DAQuiri::SettingType::boolean);
  EXPECT_EQ(DAQuiri::to_string(t), "boolean");

  t = DAQuiri::from_string("binary");
  EXPECT_EQ(t, DAQuiri::SettingType::binary);
  EXPECT_EQ(DAQuiri::to_string(t), "binary");

  t = DAQuiri::from_string("integer");
  EXPECT_EQ(t, DAQuiri::SettingType::integer);
  EXPECT_EQ(DAQuiri::to_string(t), "integer");

  t = DAQuiri::from_string("pattern");
  EXPECT_EQ(t, DAQuiri::SettingType::pattern);
  EXPECT_EQ(DAQuiri::to_string(t), "pattern");

  t = DAQuiri::from_string("floating");
  EXPECT_EQ(t, DAQuiri::SettingType::floating);
  EXPECT_EQ(DAQuiri::to_string(t), "floating");

  t = DAQuiri::from_string("precise");
  EXPECT_EQ(t, DAQuiri::SettingType::precise);
  EXPECT_EQ(DAQuiri::to_string(t), "precise");

  t = DAQuiri::from_string("text");
  EXPECT_EQ(t, DAQuiri::SettingType::text);
  EXPECT_EQ(DAQuiri::to_string(t), "text");

  t = DAQuiri::from_string("time");
  EXPECT_EQ(t, DAQuiri::SettingType::time);
  EXPECT_EQ(DAQuiri::to_string(t), "time");

  t = DAQuiri::from_string("duration");
  EXPECT_EQ(t, DAQuiri::SettingType::duration);
  EXPECT_EQ(DAQuiri::to_string(t), "duration");

  t = DAQuiri::from_string("command");
  EXPECT_EQ(t, DAQuiri::SettingType::command);
  EXPECT_EQ(DAQuiri::to_string(t), "command");

  t = DAQuiri::from_string("indicator");
  EXPECT_EQ(t, DAQuiri::SettingType::indicator);
  EXPECT_EQ(DAQuiri::to_string(t), "indicator");

  t = DAQuiri::from_string("menu");
  EXPECT_EQ(t, DAQuiri::SettingType::menu);
  EXPECT_EQ(DAQuiri::to_string(t), "menu");
}

TEST_F(SettingMeta, IsType)
{
  DAQuiri::SettingMeta m("a", DAQuiri::SettingType::integer);
  EXPECT_FALSE(m.is(DAQuiri::SettingType::floating));
  EXPECT_TRUE(m.is(DAQuiri::SettingType::integer));
}

TEST_F(SettingMeta, IsNumeric)
{
  EXPECT_FALSE(DAQuiri::SettingMeta().is_numeric());

  EXPECT_TRUE(DAQuiri::SettingMeta("", DAQuiri::SettingType::integer).is_numeric());
  EXPECT_TRUE(DAQuiri::SettingMeta("", DAQuiri::SettingType::floating).is_numeric());
  EXPECT_TRUE(DAQuiri::SettingMeta("", DAQuiri::SettingType::precise).is_numeric());

  EXPECT_FALSE(DAQuiri::SettingMeta("", DAQuiri::SettingType::boolean).is_numeric());
  EXPECT_FALSE(DAQuiri::SettingMeta("", DAQuiri::SettingType::binary).is_numeric());
  EXPECT_FALSE(DAQuiri::SettingMeta("", DAQuiri::SettingType::pattern).is_numeric());
  EXPECT_FALSE(DAQuiri::SettingMeta("", DAQuiri::SettingType::text).is_numeric());

  EXPECT_FALSE(DAQuiri::SettingMeta("", DAQuiri::SettingType::time).is_numeric());
  EXPECT_FALSE(DAQuiri::SettingMeta("", DAQuiri::SettingType::duration).is_numeric());
  EXPECT_FALSE(DAQuiri::SettingMeta("", DAQuiri::SettingType::command).is_numeric());
  EXPECT_FALSE(DAQuiri::SettingMeta("", DAQuiri::SettingType::menu).is_numeric());
  EXPECT_FALSE(DAQuiri::SettingMeta("", DAQuiri::SettingType::indicator).is_numeric());
  EXPECT_FALSE(DAQuiri::SettingMeta("", DAQuiri::SettingType::stem).is_numeric());
}

TEST_F(SettingMeta, Init)
{
  DAQuiri::SettingMeta m;
  EXPECT_EQ(m.id(), "");
  EXPECT_EQ(m.type(), DAQuiri::SettingType::none);

  m = DAQuiri::SettingMeta("a", DAQuiri::SettingType::integer);
  EXPECT_EQ(m.id(), "a");
  EXPECT_EQ(m.type(), DAQuiri::SettingType::integer);
  EXPECT_EQ(m.get_string("name", ""), "a");

  m = DAQuiri::SettingMeta("a", DAQuiri::SettingType::integer, "b");
  EXPECT_EQ(m.id(), "a");
  EXPECT_EQ(m.type(), DAQuiri::SettingType::integer);
  EXPECT_EQ(m.get_string("name", ""), "b");
}

TEST_F(SettingMeta, Meaningful)
{
  DAQuiri::SettingMeta m("a", DAQuiri::SettingType::integer);
  EXPECT_TRUE(m.meaningful());

  m = m.stripped();
  EXPECT_FALSE(m.meaningful());
}

TEST_F(SettingMeta, Flags)
{
  DAQuiri::SettingMeta m;

  m.set_flag("a");
  EXPECT_TRUE(m.has_flag("a"));
  m.remove_flag("a");
  EXPECT_FALSE(m.has_flag("a"));

  EXPECT_FALSE(m.has_flag("b"));
  m.remove_flag("b");
  EXPECT_FALSE(m.has_flag("b"));

  m.set_flags({"c", "d"});
  EXPECT_TRUE(m.has_flag("c"));
  EXPECT_TRUE(m.has_flag("d"));
}

TEST_F(SettingMeta, Enum)
{
  DAQuiri::SettingMeta m;

  EXPECT_FALSE(m.has_enum(1));
  m.set_enum(1, "a");
  EXPECT_TRUE(m.has_enum(1));
  EXPECT_EQ(m.enum_name(1), "a");

  EXPECT_FALSE(m.has_enum(2));
  m.set_enum(2, "b");
  EXPECT_TRUE(m.has_enum(2));

  EXPECT_EQ(m.enum_names(), std::list<std::string>({"a", "b"}));

  EXPECT_EQ(m.enum_name(42), "");
}

TEST_F(SettingMeta, SetEnums)
{
  DAQuiri::SettingMeta m;

  m.set_enums(0, {"a", "b", "c"});
  EXPECT_EQ(m.enum_name(0), "a");
  EXPECT_EQ(m.enum_name(1), "b");
  EXPECT_EQ(m.enum_name(2), "c");

  m.set_enums(42, {"x", "y", "z"});
  EXPECT_EQ(m.enum_name(42), "x");
  EXPECT_EQ(m.enum_name(43), "y");
  EXPECT_EQ(m.enum_name(44), "z");
}

TEST_F(SettingMeta, EnumMap)
{
  DAQuiri::SettingMeta m;

  m.set_enum(1, "a");
  m.set_enum(2, "b");

  auto em = m.enum_map();
  EXPECT_EQ(em.size(), 2);
  EXPECT_EQ(em[1], "a");
  EXPECT_EQ(em[2], "b");
}


TEST_F(SettingMeta, Values)
{
  DAQuiri::SettingMeta m;

  EXPECT_EQ(m.get_string("a", "z"), "z");
  m.set_val("a", "aa");
  EXPECT_EQ(m.get_string("a", "z"), "aa");

  EXPECT_EQ(m.get_num("b", DAQuiri::integer_t(2)), 2);
  m.set_val("b", DAQuiri::integer_t(1));
  EXPECT_EQ(m.get_num("b", DAQuiri::integer_t(2)), 1);

  EXPECT_EQ(m.get_num("c", DAQuiri::floating_t(2.2)), 2.2);
  m.set_val("c", DAQuiri::floating_t(1.1));
  EXPECT_EQ(m.get_num("c", DAQuiri::floating_t(2.2)), 1.1);
}

TEST_F(SettingMeta, MinMaxStep)
{
  DAQuiri::SettingMeta m("", DAQuiri::SettingType::floating);

  EXPECT_EQ(m.min<DAQuiri::integer_t>(), -std::numeric_limits<DAQuiri::integer_t>::max());
  m.set_val("min", DAQuiri::integer_t(-42));
  EXPECT_EQ(m.min<DAQuiri::integer_t>(), -42);

  EXPECT_EQ(m.max<DAQuiri::integer_t>(), std::numeric_limits<DAQuiri::integer_t>::max());
  m.set_val("max", DAQuiri::integer_t(42));
  EXPECT_EQ(m.max<DAQuiri::integer_t>(), 42);

  EXPECT_EQ(m.step<DAQuiri::floating_t>(), 1);
  m.set_val("step", DAQuiri::floating_t(0.5));
  EXPECT_EQ(m.step<DAQuiri::floating_t>(), 0.5);
}

TEST_F(SettingMeta, Bounds)
{
  DAQuiri::SettingMeta m("", DAQuiri::SettingType::integer);
  m.set_bounds(2, 10);
  EXPECT_EQ(m.min<DAQuiri::integer_t>(), 2);
  EXPECT_EQ(m.max<DAQuiri::integer_t>(), 10);

  DAQuiri::SettingMeta m2("", DAQuiri::SettingType::floating);
  m2.set_bounds(3.0, 0.5, 42.0);
  EXPECT_EQ(m2.min<DAQuiri::floating_t>(), 3.0);
  EXPECT_EQ(m2.step<DAQuiri::floating_t>(), 0.5);
  EXPECT_EQ(m2.max<DAQuiri::floating_t>(), 42.0);
}

TEST_F(SettingMeta, ValueRange)
{
  EXPECT_TRUE(DAQuiri::SettingMeta().value_range().empty());

  DAQuiri::SettingMeta i("", DAQuiri::SettingType::integer);
  EXPECT_EQ(i.value_range(), "[m\uFF1A1\uFF1AM]");
  i.set_bounds(2, 10);
  EXPECT_EQ(i.value_range(), "[2\uFF1A1\uFF1A10]");
  i.set_bounds(2, 2, 10);
  EXPECT_EQ(i.value_range(), "[2\uFF1A2\uFF1A10]");

  DAQuiri::SettingMeta f("", DAQuiri::SettingType::floating);
  EXPECT_EQ(f.value_range(), "[m\uFF1A1\uFF1AM]");
  f.set_bounds(0.5, 10.5);
  EXPECT_EQ(f.value_range(), "[0.5\uFF1A1\uFF1A10.5]");
  f.set_bounds(0.5, 0.5, 10.5);
  EXPECT_EQ(f.value_range(), "[0.5\uFF1A0.5\uFF1A10.5]");

  DAQuiri::SettingMeta p("", DAQuiri::SettingType::precise);
  EXPECT_EQ(p.value_range(), "[m\uFF1A1\uFF1AM]");
  p.set_bounds(0.5, 10.5);
  EXPECT_EQ(p.value_range(), "[0.5\uFF1A1\uFF1A10.5]");
  p.set_bounds(0.5, 0.5, 10.5);
  EXPECT_EQ(p.value_range(), "[0.5\uFF1A0.5\uFF1A10.5]");
}


TEST_F(SettingMeta, ToJson)
{
  DAQuiri::SettingMeta m("id", DAQuiri::SettingType::floating);
  m.set_enums(0, {"a", "b"});
  m.set_bounds(3.0, 0.5, 42.0);
  m.set_flags({"n", "m"});
  json j = m;

  json j2 {
      {"id", "id"},
      {"type", "floating"},
      {"items", json::array({ {{"val",0}, {"meaning", "a"}},
                              {{"val",1}, {"meaning", "b"}} })},
      {"flags", {"m", "n"}},
      {"contents", {
          {"name", "id"},
          {"min", 3.0},
          {"step", 0.5},
          {"max", 42.0}
      }}
  };

  EXPECT_EQ(j, j2);
}

TEST_F(SettingMeta, FromJson)
{
  json j {
      {"id", "id"},
      {"type", "floating"},
      {"items", json::array({ {{"val",0}, {"meaning", "a"}},
                              {{"val",1}, {"meaning", "b"}} })},
      {"flags", {"m", "n"}},
      {"contents", {
          {"name", "id"},
          {"min", 3.0},
          {"step", 0.5},
          {"max", 42.0}
      }}
  };

  DAQuiri::SettingMeta m = j;

  EXPECT_EQ(m.id(), "id");
  EXPECT_TRUE(m.is(DAQuiri::SettingType::floating));
  EXPECT_EQ(m.enum_name(0), "a");
  EXPECT_EQ(m.enum_name(1), "b");
  EXPECT_TRUE(m.has_flag("m"));
  EXPECT_TRUE(m.has_flag("n"));
  EXPECT_EQ(m.get_num("min", 0.0), 3.0);
  EXPECT_EQ(m.get_num("step", 0.0), 0.5);
  EXPECT_EQ(m.get_num("max", 0.0), 42.0);
  EXPECT_EQ(m.get_string("name", ""), "id");
}

TEST_F(SettingMeta, Debug)
{
  json j {
      {"id", "id"},
      {"type", "floating"},
      {"items", json::array({ {{"val",0}, {"meaning", "a"}},
                              {{"val",1}, {"meaning", "b"}} })},
      {"flags", {"m", "n"}},
      {"contents", {
          {"name", "id"},
          {"min", 3.0},
          {"step", 0.5},
          {"max", 42.0}
      }}
  };

  DAQuiri::SettingMeta m = j;

  MESSAGE() << "\n" << m.debug("", false) << "\n";;
}

TEST_F(SettingMeta, DebugVerbose)
{
  json j {
      {"id", "id"},
      {"type", "floating"},
      {"items", json::array({ {{"val",0}, {"meaning", "a"}},
                              {{"val",1}, {"meaning", "b"}} })},
      {"flags", {"m", "n"}},
      {"contents", {
          {"name", "id"},
          {"min", 3.0},
          {"step", 0.5},
          {"max", 42.0}
      }}
  };

  DAQuiri::SettingMeta m = j;

  MESSAGE() << "\n" << m.debug("", true) << "\n";
}
