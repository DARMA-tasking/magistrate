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

#endif
