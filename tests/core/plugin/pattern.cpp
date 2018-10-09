#include <core/plugin/pattern.h>
#include <gtest/gtest.h>

TEST(Pattern, Init)
{
  DAQuiri::Pattern e;
  ASSERT_EQ(e.threshold(), 0UL);
  ASSERT_EQ(e.gates().size(), 0UL);

  DAQuiri::Pattern e2(2, {true,true});
  ASSERT_EQ(e2.threshold(), 2UL);
  ASSERT_EQ(e2.gates(), std::vector<bool>({true,true}));
}

TEST(Pattern, Set)
{
  DAQuiri::Pattern e;

  e.set(2, {true,true});
  ASSERT_EQ(e.threshold(), 2UL);
  ASSERT_EQ(e.gates(), std::vector<bool>({true,true}));

  e.set_gates({true,false,true});
  ASSERT_EQ(e.gates(), std::vector<bool>({true,false,true}));

  e.set_threshold(1);
  ASSERT_EQ(e.threshold(), 1UL);

  e.set_threshold(3);
  ASSERT_EQ(e.threshold(), 3UL);

  e.set_threshold(4);
  ASSERT_EQ(e.threshold(), 3UL);
}

TEST(Pattern, Resize)
{
  DAQuiri::Pattern e;

  e.set(3, {true,true,true});
  e.resize(2);
  ASSERT_EQ(e.threshold(), 2UL);
  ASSERT_EQ(e.gates(), std::vector<bool>({true,true}));

  e.resize(3);
  ASSERT_EQ(e.threshold(), 2UL);
  ASSERT_EQ(e.gates(), std::vector<bool>({true,true, false}));
}

TEST(Pattern, Compare)
{
  DAQuiri::Pattern p1;
  DAQuiri::Pattern p2;

  p1.set(3, {true,true,true});
  p2.set(3, {true,true,true});
  ASSERT_TRUE(p1 == p2);
  ASSERT_FALSE(p1 != p2);

  p2.set_threshold(2);
  ASSERT_FALSE(p1 == p2);
  ASSERT_TRUE(p1 != p2);

  p2.set(3, {true, false, false});
  ASSERT_FALSE(p1 == p2);
  ASSERT_TRUE(p1 != p2);
}

TEST(Pattern, Relevant)
{
  DAQuiri::Pattern p;

  p.set(2, {true, false});
  ASSERT_TRUE(p.relevant(0));
  ASSERT_FALSE(p.relevant(1));
  ASSERT_FALSE(p.relevant(2));
}

TEST(Pattern, Print)
{
  DAQuiri::Pattern p;

  p.set(2, {true, false});
  ASSERT_EQ(p.debug(), "2+o");
}
