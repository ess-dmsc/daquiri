#include "gtest_color_print.h"
#include <core/calibration/udouble.h>

class UDouble : public TestBase
{
};

TEST_F(UDouble, Default)
{
  DAQuiri::UDouble<true> ud(0.0, 0.0);
//  EXPECT_TRUE(std::isnan(ud.value()));
//  EXPECT_TRUE(std::isnan(ud.sigma()));
//  EXPECT_EQ(ud.sigfigs(), 1);
}
