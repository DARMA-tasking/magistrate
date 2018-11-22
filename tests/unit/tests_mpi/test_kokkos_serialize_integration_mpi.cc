#if KOKKOS_ENABLED_SERDES

#include "test_harness.h"
#include "test_commons.h"
#include "test_kokkos_integration_commons.h"
#include "tests_mpi/mpi-init.h"

#include <mpich-clang39/mpi.h>

struct KokkosIntegrateTestMPI : KokkosBaseTest { };

TEST_F(KokkosIntegrateTestMPI, test_integrate_1) {
  using namespace serialization::interface;
  using DataType = Data;

  // Init test_data, check for golden status before and after serialization
  DataType test_data(DataConsTag);
  //Data::checkIsGolden(test_data);

  // Test the respect of the max rank needed for the test'
  EXPECT_EQ(MPIEnvironment::isRankValid(1), true);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0) {
    auto ret = serialize<DataType>(test_data);
    int dataSize = ret->getSize();
    MPI_Send( &dataSize, 1, MPI_INT, 1, 0, MPI_COMM_WORLD );
    char * viewBuffer = ret->getBuffer();
    MPI_Send(viewBuffer, dataSize, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
  }
  else  {
    int dataSize;
    MPI_Recv( & dataSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    char * recv = (char *) malloc(dataSize);

    MPI_Recv(recv, dataSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    auto out_data = deserialize<DataType>(recv, dataSize);
    auto const& out_data_ref = *out_data;
#if SERDES_USE_ND_COMPARE
  compareND(test_data, out_data_ref);
#else
    Data::checkIsGolden(out_data_ref);
    Data::checkIsGolden(test_data);
#endif
  }
}

///////////////////////////////////////////////////////////////////////////////
// Kokkos::View unit test with no-data (null), default constructor
/////////////////////// ////////////////////////////////////////////////////////

struct KokkosNullTestMPI : KokkosBaseTest { };

TEST_F(KokkosNullTestMPI, test_null_1) {
  using namespace serialization::interface;
  using ViewType = Kokkos::View<int*>;

  // Default construct
  ViewType test_data = {};

  auto ret = serialize<ViewType>(test_data);
  auto out = deserialize<ViewType>(std::move(ret));
}

#endif
