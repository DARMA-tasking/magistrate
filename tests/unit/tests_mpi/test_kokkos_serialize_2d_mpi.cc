#if KOKKOS_ENABLED_SERDES

#include "test_harness.h"
#include "test_commons.h"
#include "test_kokkos_2d_commons.h"
#include "tests_mpi/mpi-init.h"

#include <mpich-clang39/mpi.h>

template <typename ParamT> struct KokkosViewTest2DMPI : KokkosViewTest<ParamT> { };

TYPED_TEST_CASE_P(KokkosViewTest2DMPI);

TYPED_TEST_P(KokkosViewTest2DMPI, test_2d_any) {
  using namespace serialization::interface;

  using LayoutType = typename std::tuple_element<0,TypeParam>::type;
  using DataType   = typename std::tuple_element<1,TypeParam>::type;
  using ViewType   = Kokkos::View<DataType, LayoutType>;

  static constexpr size_t const N = 23;
  static constexpr size_t const M = 32;

  constexpr auto dyn = serdes::CountDims<ViewType,DataType>::dynamic;

  LayoutType layout = layout2d<LayoutType>(N,M);
  //dyn == 2 ? layout2d<LayoutType>(N,M) : layout1d<LayoutType>(N);
  ViewType in_view("test-2D-some-string", layout);

  init2d(in_view);

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
    compare2d(in_view, out_view_ref);
#endif
  }
}

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest2DMPI, test_2d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_2d_L, KokkosViewTest2DMPI, Test2DTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_2d_R, KokkosViewTest2DMPI, Test2DTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_2d_S, KokkosViewTest2DMPI, Test2DTypesStride);

#endif
#endif
