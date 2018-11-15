#include <gtest/gtest.h>
#include <core/util/UTF_extensions.h>

TEST(UTF_extensions, Super)
{
  auto s0 = UTF_superscript(0);
  EXPECT_EQ(s0, k_UTF_superscripts[0]);
}
