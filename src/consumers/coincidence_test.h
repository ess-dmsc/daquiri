#include "coincidence.h"
#include <gtest/gtest.h>

TEST(Coincidence, Init)
{
  DAQuiri::Coincidence e;
  ASSERT_TRUE(e.empty());

  DAQuiri::Coincidence e1(DAQuiri::Hit(), 20, 20);
  ASSERT_FALSE(e1.empty());
}

TEST(Coincidence, Range)
{
  DAQuiri::Hit h;
  h.set_native_time(10);
  DAQuiri::Coincidence e(h, 20, 40);

  ASSERT_FALSE(e.antecedent(h));
  ASSERT_TRUE(e.in_window(h));
  ASSERT_FALSE(e.past_due(h));

  h.set_native_time(5);
  ASSERT_TRUE(e.antecedent(h));
  ASSERT_FALSE(e.in_window(h));
  ASSERT_FALSE(e.past_due(h));

  h.set_native_time(30);
  ASSERT_FALSE(e.antecedent(h));
  ASSERT_TRUE(e.in_window(h));
  ASSERT_FALSE(e.past_due(h));

  h.set_native_time(31);
  ASSERT_FALSE(e.antecedent(h));
  ASSERT_FALSE(e.in_window(h));
  ASSERT_FALSE(e.past_due(h));

  h.set_native_time(50);
  ASSERT_FALSE(e.antecedent(h));
  ASSERT_FALSE(e.in_window(h));
  ASSERT_FALSE(e.past_due(h));

  h.set_native_time(51);
  ASSERT_FALSE(e.antecedent(h));
  ASSERT_FALSE(e.in_window(h));
  ASSERT_TRUE(e.past_due(h));
}

TEST(Coincidence, Add)
{
  DAQuiri::Hit h;
  h.set_native_time(10);
  DAQuiri::Coincidence e(h, 20, 40);
  ASSERT_EQ(10, e.lower_time().native());
  ASSERT_EQ(10, e.upper_time().native());

  h.set_native_time(5);
  e.add_hit(h);
  ASSERT_EQ(2, e.size());
  ASSERT_EQ(5, e.lower_time().native());
  ASSERT_EQ(10, e.upper_time().native());

  h.set_native_time(20);
  e.add_hit(h);
  ASSERT_EQ(3, e.size());
  ASSERT_EQ(5, e.lower_time().native());
  ASSERT_EQ(20, e.upper_time().native());

  h.set_native_time(30);
  e.add_hit(h);
  ASSERT_EQ(4, e.size());
  ASSERT_EQ(5, e.lower_time().native());
  ASSERT_EQ(30, e.upper_time().native());

  ASSERT_EQ(4, e.hits().size());
}
