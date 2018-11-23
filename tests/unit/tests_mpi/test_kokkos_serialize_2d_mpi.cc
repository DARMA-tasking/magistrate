#if KOKKOS_ENABLED_SERDES

#include "test_kokkos_2d_commons.h"
#include "tests_mpi/test_commons_mpi.h"

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

  serialiseDeserializeBasic<ViewType>(in_view, &compare2d<ViewType>);
}

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest2DMPI, test_2d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_2d_L, KokkosViewTest2DMPI, Test2DTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_2d_R, KokkosViewTest2DMPI, Test2DTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_2d_S, KokkosViewTest2DMPI, Test2DTypesStride);

#endif
#endif
