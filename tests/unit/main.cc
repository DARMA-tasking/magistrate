
#include <vector>

#include <gtest/gtest.h>

#if HAVE_MPI
#include "tests_mpi/mpi-init.h"
#endif
#include "test_harness.h"

#if KOKKOS_ENABLED_SERDES
#include <Kokkos_Core.hpp>
#endif

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
#if HAVE_MPI
  ::testing::AddGlobalTestEnvironment(new MPIEnvironment(&argc, argv));
#endif

  int ret = 0;
  {
#if KOKKOS_ENABLED_SERDES
    Kokkos::initialize();
#endif

    serdes::tests::unit::TestHarness::store_cmdline_args(argc, argv);
    ret = RUN_ALL_TESTS();

#if KOKKOS_ENABLED_SERDES
    Kokkos::finalize();
#endif
  }

  return ret;
}
