#include <gtest/gtest.h>
#include <core/util/bin_hex_print.h>

TEST(BinHexPrint, itobin16)
{
  EXPECT_EQ(itobin16(0), "0000000000000000");
}
