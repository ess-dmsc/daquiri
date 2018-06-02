#include <gtest/gtest.h>
#include "consumer_factory.h"

using namespace DAQuiri;

TEST(ConsumerFactory, Singleton)
{
  auto& a = ConsumerFactory::singleton();
  auto& b = ConsumerFactory::singleton();
  EXPECT_EQ(&a, &b);
}

TEST(ConsumerFactory, types)
{
  auto& cf = ConsumerFactory::singleton();
  EXPECT_TRUE(cf.types().empty());
}