#include "hit.h"
#include <gtest/gtest.h>

TEST(Hit, Init)
{
  DAQuiri::Hit h;
  ASSERT_EQ(-1, h.source_channel());
  ASSERT_EQ(DAQuiri::TimeStamp(), h.timestamp());
  ASSERT_EQ(0, h.value_count());
  ASSERT_EQ(0, h.trace_count());


}
