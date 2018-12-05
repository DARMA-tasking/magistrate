#ifndef TEST_COMMONS_MPI_H
#define TEST_COMMONS_MPI_H

#include "test_commons.h"
#include "test_kokkos_0d_commons.h"
#include "tests_mpi/mpi-init.h"

#include <functional>

#include <mpi.h>

namespace  {
template <typename T>
void serializeAnyMPI(T& view, std::function<void(T const&,T const&)> compare) {
  using namespace serialization::interface;

  // Test the respect of the max rank needed for the test'
  EXPECT_EQ(MPIEnvironment::isRankValid(1), true);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0) {
    auto ret = serialize<T>(view);
    int viewSize = ret->getSize();
    MPI_Send( &viewSize, 1, MPI_INT, 1, 0, MPI_COMM_WORLD );
    char * viewBuffer = ret->getBuffer();
    MPI_Send(viewBuffer, viewSize, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
  }
  else  {
    int viewSize;
    MPI_Recv( & viewSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    char * recv = new char[viewSize];

    MPI_Recv(recv, viewSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    auto out_view = deserialize<T>(recv, viewSize);
    auto const& out_view_ref = *out_view;

#if SERDES_USE_ND_COMPARE
    compareND(view, out_view_ref);
#else
    compare(view, out_view_ref);
#endif
  }
}
} // end namespace

#endif // TEST_COMMONS_MPI_H
