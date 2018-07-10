#include <gtest/gtest.h>
#include <core/producer_factory.h>

using namespace DAQuiri;

TEST(ProducerFactory, Singleton)
{
  auto& a = ProducerFactory::singleton();
  auto& b = ProducerFactory::singleton();
  EXPECT_EQ(&a, &b);
}

TEST(ProducerFactory, types)
{
  auto& pf = ProducerFactory::singleton();
  EXPECT_TRUE(pf.types().empty());
}
