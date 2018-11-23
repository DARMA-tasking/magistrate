#if KOKKOS_ENABLED_SERDES

#include "test_kokkos_3d_commons.h"
#include "tests_mpi/test_commons_mpi.h"

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

  serialiseDeserializeBasic<ViewType>(in_view, &compare3d<ViewType>);
}

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest3DMPI, test_3d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_3d_L, KokkosViewTest3DMPI, Test3DTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_R, KokkosViewTest3DMPI, Test3DTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_S, KokkosViewTest3DMPI, Test3DTypesStride);

#endif

#endif
