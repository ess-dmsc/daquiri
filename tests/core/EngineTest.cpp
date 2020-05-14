#include <gtest/gtest.h>
#include <core/Engine.h>

using namespace DAQuiri;

TEST(Engine, Singleton)
{
  auto& a = Engine::singleton();
  auto& b = Engine::singleton();
  EXPECT_EQ(&a, &b);
}
