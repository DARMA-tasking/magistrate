#include "test_harness.h"
#include "test_commons.h"
#include "test_kokkos_0d_commons.h"

template <typename ParamT> struct KokkosViewTest0D : KokkosViewTest<ParamT> { };

TYPED_TEST_CASE_P(KokkosViewTest0D);

TYPED_TEST_P(KokkosViewTest0D, test_0d_any) {
  using namespace serialization::interface;

  using DataType          = TypeParam;
  using ViewType          = Kokkos::View<DataType>;
  using NonConstT         = typename ViewType::traits::non_const_data_type;
  using NonConstViewType  = Kokkos::View<NonConstT>;
  using ConstT            = typename ViewType::traits::const_data_type;
  using ConstViewType     = Kokkos::View<ConstT>;

  static constexpr size_t const N = 241;

  NonConstViewType in_view("test");

  init0d(in_view);

  if(std::is_same<NonConstViewType, ViewType>::value)
  {
    auto ret = serialize<NonConstViewType>(in_view);
    auto out_view = deserialize<NonConstViewType>(ret->getBuffer(), ret->getSize());
    auto const& out_view_ref = *out_view;
    #if SERDES_USE_ND_COMPARE
      compareND(in_view, out_view_ref);
    #else
      compare0d(in_view, out_view_ref);
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
     compare0d(const_in_view, out_view_ref);
    #endif
  }
}

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest0D, test_0d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_0d, KokkosViewTest0D, Test0DTypes);

#endif
