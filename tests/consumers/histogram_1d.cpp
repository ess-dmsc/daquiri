#include "gtest_color_print.h"
#include "histogram_1d.h"

class Histogram1D : public TestBase
{
  protected:
    DAQuiri::Histogram1D h;
};

TEST_F(Histogram1D, Init)
{
  EXPECT_FALSE(h.changed());
  EXPECT_EQ(h.type(), "Histogram 1D");
}

