#include <gtest/gtest.h>
#include <core/spill_dequeue.h>

using namespace DAQuiri;

TEST(SpillDeque, Init)
{
  SpillDeque sd;
  EXPECT_EQ(sd.size(), 0UL);
}

