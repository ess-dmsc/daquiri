#include <gtest/gtest.h>
#include "bin_hex_print.h"

TEST(BinHexPrint, itobin16)
{
  EXPECT_EQ(itobin16(0), "0000000000000000");
}
