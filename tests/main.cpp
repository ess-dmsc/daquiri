#include <gtest/gtest.h>
#include <h5cpp/hdf5.hpp>
#include <core/util/logger.h>

int main(int argc, char **argv)
{
  hdf5::error::Singleton::instance().auto_print(false);
  CustomLogger::initLogger(spdlog::level::trace, "unit_tests.log");

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
