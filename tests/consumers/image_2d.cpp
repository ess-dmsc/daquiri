#include <gtest/gtest.h>
#include "image_2d.h"

using namespace DAQuiri;

TEST(Image2D, Init)
{
  Image2D h;
  EXPECT_EQ(h.type(), "Image 2D");
}

