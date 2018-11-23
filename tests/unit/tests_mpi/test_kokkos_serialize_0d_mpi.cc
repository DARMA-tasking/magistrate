#if KOKKOS_ENABLED_SERDES

#include "test_kokkos_0d_commons.h"
#include "tests_mpi/test_commons_mpi.h"

template <typename ParamT> struct KokkosViewTest0DMPI : KokkosViewTest<ParamT> { };
TYPED_TEST_CASE_P(KokkosViewTest0DMPI);

TYPED_TEST_P(KokkosViewTest0DMPI, test_0d_any) {
  using namespace serialization::interface;

  using DataType = TypeParam;
  using ViewType = Kokkos::View<DataType>;

  static constexpr size_t const N = 241;

  ViewType in_view("test");

  init0d(in_view);

  serialiseDeserializeBasic<ViewType>(in_view, &compare0d<ViewType>);
}

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest0DMPI, test_0d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_0d, KokkosViewTest0DMPI, Test0DTypes);

#endif
#endif
