#include <gtest/gtest.h>
#include <core/util/lexical_extensions.h>

TEST(lexicalExtensions, IsNumber)
{
  EXPECT_TRUE(is_number("123"));
}
