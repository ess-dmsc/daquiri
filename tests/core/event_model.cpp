#include <core/event_model.h>
#include <gtest/gtest.h>

TEST(EventModel, AddValue)
{
  DAQuiri::EventModel h;
  EXPECT_EQ("", h.debug());

  h.add_value("a", 2);
  ASSERT_EQ(1UL, h.values.size());
  ASSERT_EQ(1UL, h.value_names.size());
  ASSERT_EQ(1UL, h.name_to_val.size());
  ASSERT_EQ(0UL, h.name_to_val.at("a"));
  ASSERT_EQ(0UL, h.values[0]);
  ASSERT_EQ(2UL, h.maximum[0]);
  ASSERT_EQ("a", h.value_names.at(0));
  EXPECT_EQ("VALS a(<=2) ", h.debug());

  h.add_value("b", 7);
  ASSERT_EQ(2UL, h.values.size());
  ASSERT_EQ(2UL, h.value_names.size());
  ASSERT_EQ(2UL, h.name_to_val.size());
  ASSERT_EQ(1UL, h.name_to_val.at("b"));
  ASSERT_EQ(0UL, h.values[1]);
  ASSERT_EQ(7UL, h.maximum[1]);
  ASSERT_EQ("b", h.value_names.at(1));
  EXPECT_EQ("VALS a(<=2) b(<=7) ", h.debug());
}

TEST(EventModel, AddTrace)
{
  DAQuiri::EventModel h;
  EXPECT_EQ("", h.debug());

  h.add_trace("x", {2,3});
  ASSERT_EQ(1UL, h.traces.size());
  ASSERT_EQ(1UL, h.trace_names.size());
  ASSERT_EQ(1UL, h.name_to_trace.size());
  ASSERT_EQ(0UL, h.name_to_trace.at("x"));
  ASSERT_EQ(2UL, h.traces.at(0).size());
  ASSERT_EQ(std::vector<size_t>({2,3}), h.traces.at(0));
  ASSERT_EQ("x", h.trace_names.at(0));
  EXPECT_EQ("TRACES x( 2 3 ) ", h.debug());

  h.add_trace("y", {2,5,7});
  ASSERT_EQ(2UL, h.traces.size());
  ASSERT_EQ(2UL, h.trace_names.size());
  ASSERT_EQ(2UL, h.name_to_trace.size());
  ASSERT_EQ(1UL, h.name_to_trace.at("y"));
  ASSERT_EQ(3UL, h.traces.at(1).size());
  ASSERT_EQ(std::vector<size_t>({2,5,7}), h.traces.at(1));
  ASSERT_EQ("y", h.trace_names.at(1));
  EXPECT_EQ("TRACES x( 2 3 ) y( 2 5 7 ) ", h.debug());
}
