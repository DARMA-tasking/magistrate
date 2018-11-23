#if KOKKOS_ENABLED_SERDES

#include "test_kokkos_1d_commons.h"
#include "tests_mpi/test_commons_mpi.h"

template <typename ParamT> struct KokkosViewTest1DMPI : KokkosViewTest<ParamT> { };

TYPED_TEST_CASE_P(KokkosViewTest1DMPI);

TYPED_TEST_P(KokkosViewTest1DMPI, test_1d_any) {
  using namespace serialization::interface;

  using LayoutType = typename std::tuple_element<0,TypeParam>::type;
  using DataType   = typename std::tuple_element<1,TypeParam>::type;
  using ViewType   = Kokkos::View<DataType, LayoutType>;

  static constexpr size_t const N = 241;

  LayoutType layout = layout1d<LayoutType>(N);
  ViewType in_view("test", layout);

  init1d(in_view);

  serialiseDeserializeBasic<ViewType>(in_view, &compare1d<ViewType>);
}


REGISTER_TYPED_TEST_CASE_P(KokkosViewTest1DMPI, test_1d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_1d_L, KokkosViewTest1DMPI, Test1DTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_1d_R, KokkosViewTest1DMPI, Test1DTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_1d_S, KokkosViewTest1DMPI, Test1DTypesStride);

#endif

///////////////////////////////////////////////////////////////////////////////
// Kokkos::DynamicView Unit Tests: dynamic view is restricted to 1-D in kokkos
///////////////////////////////////////////////////////////////////////////////

template <typename ParamT>
struct KokkosDynamicViewTestMPI : KokkosViewTest<ParamT> { };

TYPED_TEST_CASE_P(KokkosDynamicViewTestMPI);

TYPED_TEST_P(KokkosDynamicViewTestMPI, test_dynamic_1d) {
  using namespace serialization::interface;

  using DataType = TypeParam;
  using ViewType = Kokkos::Experimental::DynamicView<DataType>;

  static constexpr std::size_t const N = 64;
  static constexpr unsigned const min_chunk = 8;
  static constexpr unsigned const max_extent = 1024;

  ViewType in_view("my-dynamic-view", min_chunk, max_extent);
  in_view.resize_serial(N);

  init1d(in_view);

  serialiseDeserializeBasic<ViewType>(in_view, &compare1d<ViewType>);
}

REGISTER_TYPED_TEST_CASE_P(KokkosDynamicViewTestMPI, test_dynamic_1d);

#if DO_UNIT_TESTS_FOR_VIEW
INSTANTIATE_TYPED_TEST_CASE_P(
  test_dynamic_view_1, KokkosDynamicViewTestMPI, DynamicTestTypes
);
#endif

#endif
