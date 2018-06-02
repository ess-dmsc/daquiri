#include <gtest/gtest.h>
#include "uncertain.h"

using namespace DAQuiri;

TEST(UncertainDouble, Init)
{
  UncertainDouble ud;
  EXPECT_EQ(ud.sigfigs(), 0);
}
