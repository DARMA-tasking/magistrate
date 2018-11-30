
#include <vector>

#include <gtest/gtest.h>

#if HAVE_MPI
#include "tests_mpi/mpi-init.h"
#endif
#include "test_harness.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
#if HAVE_MPI
  ::testing::AddGlobalTestEnvironment(new MPIEnvironment(&argc, argv));
#endif

  serdes::tests::unit::TestHarness::store_cmdline_args(argc, argv);
  int const ret = RUN_ALL_TESTS();

  return ret;
}
