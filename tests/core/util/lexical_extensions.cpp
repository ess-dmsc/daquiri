#include <gtest/gtest.h>
#include "lexical_extensions.h"

TEST(lexicalExtensions, IsNumber)
{
  EXPECT_TRUE(is_number("123"));
}
