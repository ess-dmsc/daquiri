#include <gtest/gtest.h>
#include <core/ConsumerMetadata.h>

using namespace DAQuiri;

TEST(ConsumerMetadata, Init)
{
  ConsumerMetadata m;
  EXPECT_TRUE(m.type().empty());
}
