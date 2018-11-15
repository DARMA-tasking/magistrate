#if KOKKOS_ENABLED_SERDES

#include "test_harness.h"
#include "test_commons.h"

///////////////////////////////////////////////////////////////////////////////
// Kokkos::View Integration Tests with other elements
///////////////////////////////////////////////////////////////////////////////

struct BaseData { int a = 10; };

static struct DataConsTagType { } DataConsTag { };

static constexpr int const gold_val1 = 10;
static constexpr int const gold_val2 = 20;
static constexpr int const gold_val3 = 29;

static constexpr std::size_t const d1_a = 100;
static constexpr std::size_t const d2_a = 8, d2_b = 7;
static constexpr std::size_t const d3_a = 2, d3_b = 7, d3_c = 4;
static constexpr std::size_t const d4_a = 2;

struct Data : BaseData {
  using AtomicTrait      = ::Kokkos::MemoryTraits<Kokkos::Atomic>;
  using Kokkos_ViewType0 = ::Kokkos::View<int,      AtomicTrait>;
  using Kokkos_ViewType1 = ::Kokkos::View<int*,     Kokkos::LayoutLeft>;
  using Kokkos_ViewType2 = ::Kokkos::View<double**, Kokkos::LayoutRight>;
  using Kokkos_ViewType3 = ::Kokkos::View<float***, AtomicTrait>;
  using Kokkos_ViewType4 = ::Kokkos::View<int*[2]>;
  using DimType          = typename Kokkos_ViewType1::size_type;

  Data() = default;

  explicit Data(DataConsTagType)
    : val1(gold_val1), val2(gold_val2), vec({gold_val3,gold_val3+1,gold_val3+2})
  {
    Kokkos_ViewType0 v0_tmp("v0_tmp");
    Kokkos_ViewType1 v1_tmp("v1_tmp",d1_a);
    Kokkos_ViewType2 v2_tmp("v2_tmp",d2_a,d2_b);
    Kokkos_ViewType3 v3_tmp("v3_tmp",d3_a,d3_b,d3_c);
    Kokkos_ViewType4 v4_tmp("v4_tmp",d4_a);

    v0_tmp.operator()() = v0val();

   for (DimType i = 0; i < d1_a; i++) {
      v1_tmp.operator()(i) = v1val(i);
    }
    for (DimType i = 0; i < d2_a; i++) {
      for (DimType j = 0; j < d2_b; j++) {
        v2_tmp.operator()(i,j) = v2val(i,j);
      }
    }
    for (DimType i = 0; i < d3_a; i++) {
      for (DimType j = 0; j < d3_b; j++) {
        for (DimType k = 0; k < d3_c; k++) {
          v3_tmp.operator()(i,j,k) = v3val(i,j,k);//d3_a*d3_b*i + j*d3_a + k;
        }
      }
    }
    for (DimType i = 0; i < d4_a; i++) {
      v4_tmp.operator()(i,0) = v4val(i,0);//d4_a*i*2 + 1;
      v4_tmp.operator()(i,1) = v4val(i,1);//d4_a*i*2 + 2;
    }
    v0 = v0_tmp;
    v1 = v1_tmp;
    v2 = v2_tmp;
    v3 = v3_tmp;
    v4 = v4_tmp;
  }

  /* Generators for creating expected data values */
  static int v0val() {
    return d1_a * 29;
  }
  static int v1val(DimType i) {
    return d1_a * i;
  }
  static double v2val(DimType i, DimType j) {
    return d2_a*i + j;
  }
  static float v3val(DimType i, DimType j, DimType k) {
    return d3_a*d3_b*i + j*d3_a + k;
  }
  static int v4val(DimType i, DimType j) {
    return j == 0 ? d4_a*i*2 + 1 : d4_a*i*2 + 2;
  }

  /* Check that all values are golden and match with expected generators */
  static void checkIsGolden(Data const& in) {
    EXPECT_EQ(in.val1,gold_val1);
    EXPECT_EQ(in.val2,gold_val2);
    EXPECT_EQ(in.vec.size(),3);
    EXPECT_EQ(in.vec[0],gold_val3+0);
    EXPECT_EQ(in.vec[1],gold_val3+1);
    EXPECT_EQ(in.vec[2],gold_val3+2);
    EXPECT_EQ(in.v0.size(),1);
    EXPECT_EQ(in.v1.size(),d1_a);
    EXPECT_EQ(in.v2.size(),d2_a*d2_b);
    EXPECT_EQ(in.v3.size(),d3_a*d3_b*d3_c);
    EXPECT_EQ(in.v4.size(),d4_a*2);

    EXPECT_EQ(in.v0.operator()(), v0val());
    for (DimType i = 0; i < d1_a; i++) {
      EXPECT_EQ(in.v1.operator()(i), v1val(i));
    }
    for (DimType i = 0; i < d2_a; i++) {
      for (DimType j = 0; j < d2_b; j++) {
        EXPECT_EQ(in.v2.operator()(i,j), v2val(i,j));
      }
    }
    for (DimType i = 0; i < d3_a; i++) {
      for (DimType j = 0; j < d3_b; j++) {
        for (DimType k = 0; k < d3_c; k++) {
          EXPECT_EQ(in.v3.operator()(i,j,k), v3val(i,j,k));
        }
      }
    }
    for (DimType i = 0; i < d4_a; i++) {
      EXPECT_EQ(in.v4.operator()(i,0), v4val(i,0));
      EXPECT_EQ(in.v4.operator()(i,1), v4val(i,1));
    }
  }

  template <typename SerializerT>
  friend void serdes::serialize(SerializerT& s, Data& data);

public:
  std::vector<int> vec = {};
  int val1 = 1, val2 = 2;
  Kokkos_ViewType0 v0;
  Kokkos_ViewType1 v1;
  Kokkos_ViewType2 v2;
  Kokkos_ViewType3 v3;
  Kokkos_ViewType4 v4;
};

namespace serdes {

template <typename SerializerT>
void serialize(SerializerT& s, BaseData& base) {
  s | base.a;
}

template <typename SerializerT>
void serialize(SerializerT& s, Data& data) {
  BaseData& base_cls = static_cast<BaseData&>(data);
  s | base_cls;
  s | data.vec;
  s | data.val1 | data.val2;
  s | data.v0 | data.v1 | data.v2 | data.v3 | data.v4;
}

} /* end namespace serdes */

struct KokkosBaseTest : virtual testing::Test {
  virtual void SetUp() override {
    Kokkos::initialize();
  }
  virtual void TearDown() override {
    Kokkos::finalize();
  }
};

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
// Kokkos::DynamicView Unit Tests: dynamic view is restricted to 1-D in kokkos
///////////////////////////////////////////////////////////////////////////////

template <typename ParamT>
struct KokkosDynamicViewTest : KokkosViewTest<ParamT> { };

TYPED_TEST_CASE_P(KokkosDynamicViewTest);

TYPED_TEST_P(KokkosDynamicViewTest, test_dynamic_1d) {
  using namespace serialization::interface;

  using DataType = TypeParam;
  using ViewType = Kokkos::Experimental::DynamicView<DataType>;

  static constexpr std::size_t const N = 64;
  static constexpr unsigned const min_chunk = 8;
  static constexpr unsigned const max_extent = 1024;

  ViewType in_view("my-dynamic-view", min_chunk, max_extent);
  in_view.resize_serial(N);

  // std::cout << "INIT size=" << in_view.size() << std::endl;

  init1d(in_view);

  auto ret = serialize<ViewType>(in_view);
  auto out_view = deserialize<ViewType>(ret->getBuffer(), ret->getSize());
  auto const& out_view_ref = *out_view;

  /*
   *  Uncomment these lines (one or both) to test the failure mode: ensure the
   *  view equality test code is operating correctly.
   *
   *   out_view_ref(3) = 10;
   *   out_view->resize_serial(N-1);
   *
   */

#if SERDES_USE_ND_COMPARE
  compareND(in_view, out_view_ref);
#else
  compare1d(in_view, out_view_ref);
#endif
}

REGISTER_TYPED_TEST_CASE_P(KokkosDynamicViewTest, test_dynamic_1d);

using DynamicTestTypes = testing::Types<
  int      *,
  double   *,
  float    *,
  int32_t  *,
  int64_t  *,
  unsigned *,
  long     *,
  long long*
>;

INSTANTIATE_TYPED_TEST_CASE_P(
  test_dynamic_view_1, KokkosDynamicViewTest, DynamicTestTypes
);

#endif
