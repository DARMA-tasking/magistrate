#if KOKKOS_ENABLED_SERDES

#include "test_harness.h"
#include "test_commons.h"

#include "test_kokkos_2d_commons.h"

template <typename ParamT> struct KokkosViewTest2D : KokkosViewTest<ParamT> { };

TYPED_TEST_CASE_P(KokkosViewTest2D);

TYPED_TEST_P(KokkosViewTest2D, test_2d_any) {
  using namespace serialization::interface;

  using LayoutType        = typename std::tuple_element<0,TypeParam>::type;
  using DataType          = typename std::tuple_element<1,TypeParam>::type;
  using ViewType          = Kokkos::View<DataType, LayoutType>;
  using NonConstT         = typename ViewType::traits::non_const_data_type;
  using NonConstViewType  = Kokkos::View<NonConstT, LayoutType>;
  using ConstT         = typename ViewType::traits::const_data_type;
  using ConstViewType  = Kokkos::View<ConstT, LayoutType>;
  static constexpr size_t const N = 23;
  static constexpr size_t const M = 32;

  LayoutType layout = layout2d<LayoutType>(N,M);

  NonConstViewType in_view("test-2D-some-string", layout);
  init2d(in_view);

  if(std::is_same<NonConstViewType, ViewType>::value)
  {
    auto ret = serialize<NonConstViewType>(in_view);
    auto out_view = deserialize<NonConstViewType>(ret->getBuffer(), ret->getSize());
    auto const& out_view_ref = *out_view;
    #if SERDES_USE_ND_COMPARE
      compareND(in_view, out_view_ref);
    #else
      compare2d(in_view, out_view_ref);
    #endif
  }
  else
  {
    ConstViewType const_in_view = in_view;
    auto ret = serialize<ConstViewType>(const_in_view);
    auto out_view = deserialize<ConstViewType>(ret->getBuffer(), ret->getSize());
    auto const& out_view_ref = *out_view;

      // Uncomment to make the test failed
    #if SERDES_USE_ND_COMPARE
     compareND(const_in_view, out_view_ref);
    #else
     compare2d(const_in_view, out_view_ref);
    #endif
  }

}

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest2D, test_2d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_2d_L,   KokkosViewTest2D, Test2DTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_2d_R,   KokkosViewTest2D, Test2DTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_2d_S,   KokkosViewTest2D, Test2DTypesStride);
INSTANTIATE_TYPED_TEST_CASE_P(test_2d_L_C, KokkosViewTest2D, Test2DConstTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_2d_R_C, KokkosViewTest2D, Test2DConstTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_2d_S_C, KokkosViewTest2D, Test2DConstTypesStride);

#endif
#endif
