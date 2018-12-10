#if KOKKOS_ENABLED_SERDES

#include "test_harness.h"
#include "test_commons.h"

#include "test_kokkos_integration_commons.h"

struct KokkosIntegrateTest : KokkosBaseTest { };

TEST_F(KokkosIntegrateTest, test_integrate_1) {
  using namespace serialization::interface;
  using DataType = Data;

  // Init test_data, check for golden status before and after serialization
  DataType test_data(DataConsTag);
  Data::checkIsGolden(test_data);

  auto ret = serialize<DataType>(test_data);
  auto out = deserialize<DataType>(std::move(ret));

  Data::checkIsGolden(*out);
  Data::checkIsGolden(test_data);
}

///////////////////////////////////////////////////////////////////////////////
// Kokkos::View unit test with no-data (null), default constructor
/////////////////////// ////////////////////////////////////////////////////////

struct KokkosNullTest : KokkosBaseTest { };

TEST_F(KokkosNullTest, test_null_1) {
  using namespace serialization::interface;
  using ViewType = Kokkos::View<int*>;

  // Default construct
  ViewType test_data = {};

  auto ret = serialize<ViewType>(test_data);
  deserialize<ViewType>(std::move(ret));
}

///////////////////////////////////////////////////////////////////////////////
// Kokkos::View unit test with view of view
/////////////////////// ////////////////////////////////////////////////////////

struct KokkosViewOfVIewTest : KokkosBaseTest { };

TEST_F(KokkosViewOfVIewTest, test_view_of_view_init_1) {
  using namespace serialization::interface;
  using ViewType = Kokkos::View<Kokkos::View<double*>[3]>;

  // Default construct
  ViewType test_data = Kokkos::View<Kokkos::View<double*>[3]>("test");
  test_data(0) = Kokkos::View<double*>();
  test_data(1) = Kokkos::View<double*>();
  test_data(2) = Kokkos::View<double*>();

  auto ret = serialize<ViewType>(test_data);
  auto out = deserialize<ViewType>(std::move(ret));
}

TEST_F(KokkosViewOfVIewTest, test_view_of_view_init_2) {
  using namespace serialization::interface;
  using ViewType = Kokkos::View<Kokkos::View<double*>[3]>;

  // Default construct
  ViewType test_data = Kokkos::View<Kokkos::View<double*>[3]>("test");

  auto ret = serialize<ViewType>(test_data);
  auto out = deserialize<ViewType>(std::move(ret));
}

TEST_F(KokkosViewOfVIewTest, test_view_of_view_uninit_1) {
  using namespace serialization::interface;
  using ViewType = Kokkos::View<Kokkos::View<double*>[3]>;

  // Default construct
  ViewType test_data;
  test_data(0).label();
}

#endif
