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
  using ViewType          = Kokkos::View<DataType, LayoutType>;
  using NonConstT         = typename ViewType::traits::non_const_data_type;
  using NonConstViewType  = Kokkos::View<NonConstT, LayoutType>;
  using ConstT         = typename ViewType::traits::const_data_type;
  using ConstViewType  = Kokkos::View<ConstT, LayoutType>;
  static constexpr size_t const N = 5;
  static constexpr size_t const M = 17;
  static constexpr size_t const Q = 7;

  LayoutType layout = layout3d<LayoutType>(N,M,Q);
  NonConstViewType in_view("test-3D-some-string", layout);

  init3d(in_view);

  if(std::is_same<NonConstViewType, ViewType>::value)
  {
    serialiseDeserializeBasic<NonConstViewType>(in_view, &compare3d<NonConstViewType>);

  }
  else
  {
    ConstViewType const_in_view = in_view;
    serialiseDeserializeBasic<ConstViewType>(const_in_view, &compare3d<ConstViewType>);

  }
}

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest3D, test_3d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_3d_L, KokkosViewTest3D, Test3DTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_R, KokkosViewTest3D, Test3DTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_S, KokkosViewTest3D, Test3DTypesStride);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_L_C, KokkosViewTest3D, Test3DConstTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_R_C, KokkosViewTest3D, Test3DConstTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_S_C, KokkosViewTest3D, Test3DConstTypesStride);

#endif

#endif
