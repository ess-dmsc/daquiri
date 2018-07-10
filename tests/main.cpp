#include <gtest/gtest.h>
#include <h5cpp/hdf5.hpp>

int main(int argc, char **argv)
{
  hdf5::error::Singleton::instance().auto_print(false);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
