#include <gtest/gtest.h>
#include <core/SpillDequeue.h>

using namespace DAQuiri;

TEST(SpillDeque, Init)
{
  SpillDeque sd;
  EXPECT_EQ(sd.size(), 0UL);
}
