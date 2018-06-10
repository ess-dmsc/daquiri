#include <gtest/gtest.h>
#include <core/calibration/parameter.h>

using namespace DAQuiri;

TEST(Parameter, Init)
{
  FitParam fp;
  EXPECT_FALSE(fp.fixed());
}
