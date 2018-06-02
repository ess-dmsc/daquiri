#include <gtest/gtest.h>
#include "consumer_metadata.h"

using namespace DAQuiri;

TEST(ConsumerMetadata, Init)
{
  ConsumerMetadata m;
  EXPECT_TRUE(m.type().empty());
}
