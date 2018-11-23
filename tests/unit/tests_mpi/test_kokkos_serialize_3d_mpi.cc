#if KOKKOS_ENABLED_SERDES

#include "test_harness.h"
#include "test_commons.h"
#include "test_kokkos_3d_commons.h"

#include "tests_mpi/mpi-init.h"

#include <mpich-clang39/mpi.h>

template <typename ParamT> struct KokkosViewTest3DMPI : KokkosViewTest<ParamT> { };

TYPED_TEST_CASE_P(KokkosViewTest3DMPI);

TYPED_TEST_P(KokkosViewTest3DMPI, test_3d_any) {
  using namespace serialization::interface;

  using LayoutType = typename std::tuple_element<0,TypeParam>::type;
  using DataType   = typename std::tuple_element<1,TypeParam>::type;
  using ViewType   = Kokkos::View<DataType, LayoutType>;

  static constexpr size_t const N = 5;
  static constexpr size_t const M = 17;
  static constexpr size_t const Q = 7;

  constexpr auto dyn = serdes::CountDims<ViewType,DataType>::dynamic;

  LayoutType layout = layout3d<LayoutType>(N,M,Q);
  ViewType in_view("test-2D-some-string", layout);

  init3d(in_view);

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

    /*
     *  Uncomment this line to test the failure mode: ensure the view equality
     *  code is operating correctly.
     *
     *   out_view->operator()(3,1,0) = 1283;
     *
     */

#if SERDES_USE_ND_COMPARE
  compareND(in_view, out_view_ref);
#else
  compare3d(in_view, out_view_ref);
#endif
  }
}

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest3DMPI, test_3d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_3d_L, KokkosViewTest3DMPI, Test3DTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_R, KokkosViewTest3DMPI, Test3DTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_S, KokkosViewTest3DMPI, Test3DTypesStride);

#endif

#endif
