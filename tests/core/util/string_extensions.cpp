#include "gtest_color_print.h"
#include <core/util/string_extensions.h>

class StringExtensions : public TestBase
{
};

TEST_F(StringExtensions, ltrim)
{
  std::string a{"\n\t abc\n\t "};
  ltrim(a);
  EXPECT_EQ(a, "abc\n\t ");
}

TEST_F(StringExtensions, rtrim)
{
  std::string a{"\n\t abc\n\t "};
  rtrim(a);
  EXPECT_EQ(a, "\n\t abc");
}

TEST_F(StringExtensions, trim)
{
  std::string a{"\n\t abc\n\t "};
  trim(a);
  EXPECT_EQ(a, "abc");
}

TEST_F(StringExtensions, ltrim_copy)
{
  std::string a{"\n\t abc\n\t "};
  EXPECT_EQ(ltrim_copy(a), "abc\n\t ");
}

TEST_F(StringExtensions, rtrim_copy)
{
  std::string a{"\n\t abc\n\t "};
  EXPECT_EQ(rtrim_copy(a), "\n\t abc");
}

TEST_F(StringExtensions, trim_copy)
{
  std::string a{"\n\t abc\n\t "};
  EXPECT_EQ(trim_copy(a), "abc");
}

TEST_F(StringExtensions, trim_all)
{
  std::string a{"\n\t a\n\t b\n\t c\n\t d\n\t "};
  EXPECT_EQ(trim_all(a), "a b c d");
}

TEST_F(StringExtensions, join)
{
  std::vector<std::string> a{"a", "b", "c"};
  EXPECT_EQ(join(a, "+"), "a+b+c");
}
