#if KOKKOS_ENABLED_SERDES

#include "test_harness.h"
#include "test_commons.h"
#include "test_kokkos_3d_commons.h"

template <typename ParamT> struct KokkosViewTest3D : KokkosViewTest<ParamT> { };

TYPED_TEST_CASE_P(KokkosViewTest3D);

TYPED_TEST_P(KokkosViewTest3D, test_3d_any) {
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

  auto ret = serialize<ViewType>(in_view);
  auto out_view = deserialize<ViewType>(ret->getBuffer(), ret->getSize());
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

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest3D, test_3d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_3d_L, KokkosViewTest3D, Test3DTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_R, KokkosViewTest3D, Test3DTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_S, KokkosViewTest3D, Test3DTypesStride);

#endif

#endif
