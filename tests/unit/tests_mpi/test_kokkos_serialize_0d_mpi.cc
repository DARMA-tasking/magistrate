#include "test_harness.h"
#include "test_commons.h"
#include "test_kokkos_0d_commons.h"
#include "tests_mpi/mpi-init.h"

#include <mpich-clang39/mpi.h>

template <typename ParamT> struct KokkosViewTest0DMPI : KokkosViewTest<ParamT> { };
TYPED_TEST_CASE_P(KokkosViewTest0DMPI);

TYPED_TEST_P(KokkosViewTest0DMPI, test_0d_any) {
  using namespace serialization::interface;

  using DataType = TypeParam;
  using ViewType = Kokkos::View<DataType>;

  static constexpr size_t const N = 241;

  ViewType in_view("test");

  init0d(in_view);

  // Test the respect of the max rank needed for the test'
  EXPECT_EQ(MPIEnvironment::isRankValid(1), true);  

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0) {
    auto ret = serialize<ViewType>(in_view);
    int viewSize = ret->getSize();
    MPI_Send( &viewSize, 1, MPI_INT, 1, 0, MPI_COMM_WORLD );
    char * viewBuffer = ret->getBuffer();
    MPI_Send(viewBuffer, viewSize, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
  }
  else  {
    int viewSize;
    MPI_Recv( & viewSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    char * recv = (char *) malloc(viewSize);

    MPI_Recv(recv, viewSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    auto out_view = deserialize<ViewType>(recv, viewSize);
    auto const& out_view_ref = *out_view;
#if SERDES_USE_ND_COMPARE
    compareND(in_view, out_view_ref);
#else
    compare0d(in_view, out_view_ref);
#endif
  }
}

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest0DMPI, test_0d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_0d, KokkosViewTest0DMPI, Test0DTypes);

#endif
