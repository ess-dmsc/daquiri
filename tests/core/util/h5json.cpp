#include <gtest/gtest.h>
#include <core/util/h5json.h>

TEST(h5json, itobin16)
{
  auto f = hdf5::file::create("fakfile.h5", hdf5::file::AccessFlags::TRUNCATE);
  auto fr = f.root();
  EXPECT_TRUE(hdf5::require_group(fr, "blabla").is_valid());
}
