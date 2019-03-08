#include <gtest/gtest.h>
#include <h5cpp/hdf5.hpp>
#include <core/util/custom_logger.h>

int main(int argc, char **argv)
{
  hdf5::error::Singleton::instance().auto_print(false);
  CustomLogger::initLogger(Log::Severity::Debug, nullptr, "unit_tests.log");

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
