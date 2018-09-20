#include <gtest/gtest.h>
#include <core/spill.h>

using namespace DAQuiri;

TEST(Spill, Init)
{
  Spill s;
  EXPECT_EQ(s.type, Spill::Type::daq_status);
}

