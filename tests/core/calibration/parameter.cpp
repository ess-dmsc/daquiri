#include <gtest/gtest.h>
#include "parameter.h"

using namespace DAQuiri;

TEST(Parameter, Init)
{
  FitParam fp;
  EXPECT_FALSE(fp.fixed());
}
