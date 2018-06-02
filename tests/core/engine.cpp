#include <gtest/gtest.h>
#include "engine.h"

using namespace DAQuiri;

TEST(Engine, Singleton)
{
  auto& a = Engine::singleton();
  auto& b = Engine::singleton();
  EXPECT_EQ(&a, &b);
}

