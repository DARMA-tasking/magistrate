#if KOKKOS_ENABLED_SERDES

#include "test_kokkos_0d_commons.h"
#include "tests_mpi/test_commons_mpi.h"

template <typename ParamT> struct KokkosViewTest0DMPI : KokkosViewTest<ParamT> { };
TYPED_TEST_CASE_P(KokkosViewTest0DMPI);

TYPED_TEST_P(KokkosViewTest0DMPI, test_0d_any) {
  using namespace serialization::interface;

  using DataType = TypeParam;
  using ViewType = Kokkos::View<DataType>;
  using NonConstT         = typename ViewType::traits::non_const_data_type;
  using NonConstViewType  = Kokkos::View<NonConstT>;
  using ConstT         = typename ViewType::traits::const_data_type;
  using ConstViewType  = Kokkos::View<ConstT>;

  static constexpr size_t const N = 241;

  NonConstViewType in_view("test");

  init0d(in_view);

  if(std::is_same<NonConstViewType, ViewType>::value)
  {
    serialiseDeserializeBasic<NonConstViewType>(in_view, &compare0d<NonConstViewType>);
  }
  else
  {
    ConstViewType const_in_view = in_view;
    serialiseDeserializeBasic<ConstViewType>(const_in_view, &compare0d<ConstViewType>);
  }
}

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest0DMPI, test_0d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_0d, KokkosViewTest0DMPI, Test0DTypes);

#endif
#endif
