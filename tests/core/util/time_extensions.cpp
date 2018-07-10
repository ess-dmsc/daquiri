#include <gtest/gtest.h>
#include <core/util/time_extensions.h>

TEST(TimeExtensions, to_from_iso)
{
  boost::posix_time::ptime in {boost::posix_time::microsec_clock::universal_time()};
  auto str = boost::posix_time::to_iso_extended_string(in);
  auto out = from_iso_extended(str);
  EXPECT_EQ(in, out);
}
