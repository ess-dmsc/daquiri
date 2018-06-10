#include <gtest/gtest.h>
#include <core/util/json_file.h>

TEST(JsonFile, Empty)
{
  nlohmann::json in;
  to_json_file(in, "dummyfile");
  auto out = from_json_file("dummyfile");
  EXPECT_EQ(in, out);
}
