#include <core/plugin/setting.h>
#include <gtest/gtest.h>
#include <thread>

TEST(Setting, Init)
{
  DAQuiri::Setting s;
  ASSERT_FALSE(bool(s));

  s = DAQuiri::Setting("a");
  ASSERT_TRUE(bool(s));

  s = DAQuiri::Setting({"a", DAQuiri::SettingType::binary});
  ASSERT_TRUE(bool(s));
}

TEST(Setting, Indices)
{
  DAQuiri::Setting s;
  s.add_indices({0,1});
  ASSERT_TRUE(s.has_index(0));
  ASSERT_TRUE(s.has_index(1));
  ASSERT_FALSE(s.has_index(2));
  ASSERT_FALSE(s.has_index(-1));

  s.add_indices({-1,2});
  ASSERT_TRUE(s.has_index(2));
  ASSERT_TRUE(s.has_index(-1));

  s.clear_indices();
  s.set_indices({3,4});
  ASSERT_FALSE(s.has_index(0));
  ASSERT_FALSE(s.has_index(2));
  ASSERT_TRUE(s.has_index(3));
  ASSERT_TRUE(s.has_index(4));
}

TEST(Setting, Time)
{
  auto t = std::chrono::system_clock::now();
  DAQuiri::Setting s("a", t);
  ASSERT_EQ(s.time(), t);

  t = std::chrono::system_clock::now();
  s.set_time(t);
  ASSERT_EQ(s.time(), t);
}

TEST(Setting, Duration)
{
  auto t1 = std::chrono::system_clock::now();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  auto t2 = std::chrono::system_clock::now();
  auto d = t2 - t1;
  DAQuiri::Setting s("a", d);
  ASSERT_EQ(s.duration(), d);

  t1 = std::chrono::system_clock::now();
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  t2 = std::chrono::system_clock::now();
  d = t2 - t1;
  s.set_duration(d);
  ASSERT_EQ(s.duration(), d);
}

TEST(Setting, Pattern)
{
  DAQuiri::Pattern p(2, {true, true});
  DAQuiri::Setting s("a", p);
  ASSERT_EQ(s.pattern(), p);

  p = DAQuiri::Pattern(3, {true, true, true});
  s.set_pattern(p);
  ASSERT_EQ(s.pattern(), p);
}

TEST(Setting, Text)
{
  auto s = DAQuiri::Setting::text("a", "a");
  ASSERT_EQ(s.get_text(), "a");
  s.set_text("b");
  ASSERT_EQ(s.get_text(), "b");
}

TEST(Setting, Numeric)
{
  auto s = DAQuiri::Setting::floating("a", 1.23);
  ASSERT_EQ(s.get_number(), 1.23);
  s.set_number(3.21);
  ASSERT_EQ(s.get_number(), 3.21);
  s++;
  ASSERT_EQ(s.get_number(), 4.21);
  s--;
  ASSERT_EQ(s.get_number(), 3.21);

  s = DAQuiri::Setting::precise("a", PreciseFloat(1.23));
  ASSERT_EQ(s.get_number(), 1.23);
  s++;
  ASSERT_EQ(s.get_number(), 2.23);
  s--;
  ASSERT_EQ(s.get_number(), 1.23);

  s = DAQuiri::Setting::boolean("a", true);
  ASSERT_TRUE(s.triggered());
  s = DAQuiri::Setting::boolean("a", false);
  ASSERT_FALSE(s.triggered());

  s = DAQuiri::Setting::integer("a", 42);
  ASSERT_EQ(s.get_number(), 42);
  s++;
  ASSERT_EQ(s.get_number(), 43);
  s--;
  ASSERT_EQ(s.get_number(), 42);
  --s;
  ASSERT_EQ(s.get_number(), 41);
  ++s;
  ASSERT_EQ(s.get_number(), 42);

  s = DAQuiri::Setting({"a", DAQuiri::SettingType::menu});
  s.select(3);
  ASSERT_EQ(s.selection(), 3);

  s = DAQuiri::Setting({"a", DAQuiri::SettingType::command});
  s.trigger();
  ASSERT_TRUE(s.triggered());
  s.reset();
  ASSERT_FALSE(s.triggered());
}

TEST(Setting, Compare)
{
  auto a = DAQuiri::Setting::text("a", "a");
  auto b = DAQuiri::Setting::text("b", "b");
  auto aa = DAQuiri::Setting::text("a", "aa");

  ASSERT_TRUE(a.shallow_equals(aa));
  ASSERT_FALSE(a.shallow_equals(b));

  ASSERT_FALSE(a == aa);
  ASSERT_TRUE(a != aa);

  aa.set_text("a");
  ASSERT_TRUE(a == aa);
  ASSERT_FALSE(a != aa);

}

