#include "digitized_value_test.h"
#include "H5CC_Common.h"

int main(int argc, char **argv)
{
  H5CC::exceptions_off();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
