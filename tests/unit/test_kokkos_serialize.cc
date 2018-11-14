
#if KOKKOS_ENABLED_SERDES

#include "test_harness.h"

#include <serdes_headers.h>
#include <serialization_library_headers.h>
#include <container/array_serialize.h>
#include <container/view_serialize.h>
#include <container/string_serialize.h>
#include <container/vector_serialize.h>
#include <container/tuple_serialize.h>
#include <container/view_equality.h>

#include <Kokkos_Core.hpp>
#include <Kokkos_View.hpp>
#include <Kokkos_DynamicView.hpp>
#include <Kokkos_Serial.hpp>


/*
 * Compiling all the unit tests for Kokkos::View takes a long time, thus a
 * compile-time option to disable the unit tests if needed
 */
#define DO_UNIT_TESTS_FOR_VIEW 1

// By default, using manual compare...should I switch this?
#define SERDES_USE_ND_COMPARE 0

/*
 * This manual compare code should be removed once serdes::ViewEquality is fully
 * tested on target platforms
 */

// Manual 1,2,3 dimension comparison
template <typename ViewT, unsigned ndim>
static void compareInner1d(ViewT const& k1, ViewT const& k2) {
  std::cout << "compareInner1d: " << k1.extent(0) << "," << k2.extent(0) << "\n";
  EXPECT_EQ(k1.extent(0), k2.extent(0));
  for (typename ViewT::size_type i = 0; i < k1.extent(0); i++) {
    EXPECT_EQ(k1.operator()(i), k2.operator()(i));
  }
}

template <typename ViewT, unsigned ndim>
static void compareInner2d(ViewT const& k1, ViewT const& k2) {
  std::cout << "compareInner2d: " << k1.extent(0) << "," << k2.extent(0) << "\n";
  std::cout << "compareInner2d: " << k1.extent(1) << "," << k2.extent(1) << "\n";
  EXPECT_EQ(k1.extent(0), k2.extent(0));
  EXPECT_EQ(k1.extent(1), k2.extent(1));
  for (typename ViewT::size_type i = 0; i < k1.extent(0); i++) {
    for (typename ViewT::size_type j = 0; j < k1.extent(1); j++) {
      EXPECT_EQ(k1.operator()(i,j), k2.operator()(i,j));
    }
  }
}

template <typename ViewT, unsigned ndim>
static void compareInner3d(ViewT const& k1, ViewT const& k2) {
  std::cout << "compareInner3d: " << k1.extent(0) << "," << k2.extent(0) << "\n";
  std::cout << "compareInner3d: " << k1.extent(1) << "," << k2.extent(1) << "\n";
  std::cout << "compareInner3d: " << k1.extent(2) << "," << k2.extent(2) << "\n";
  EXPECT_EQ(k1.extent(0), k2.extent(0));
  EXPECT_EQ(k1.extent(1), k2.extent(1));
  EXPECT_EQ(k1.extent(2), k2.extent(2));
  for (typename ViewT::size_type i = 0; i < k1.extent(0); i++) {
    for (typename ViewT::size_type j = 0; j < k1.extent(1); j++) {
      for (typename ViewT::size_type k = 0; k < k1.extent(2); k++) {
        EXPECT_EQ(k1.operator()(i,j,k), k2.operator()(i,j,k));
      }
    }
  }
}

struct GTestEquality {
  template <typename T>
  bool operator()(T&& a, T&& b) const {
    EXPECT_EQ(a,b);
    return a == b;
  }
};

template <typename ViewT>
static void compareBasic(ViewT const& k1, ViewT const& k2) {
  using EqualityType = serdes::ViewEquality<ViewT>;
  EqualityType::template compareStaticDim<GTestEquality>(k1);
  EqualityType::template compareStaticDim<GTestEquality>(k2);
  EqualityType::template compareMeta<GTestEquality>(k1,k2);
}

// Manual 0,1,2,3 dimension comparison
template <typename ViewT>
static void compare0d(ViewT const& k1, ViewT const& k2) {
  compareBasic(k1,k2);
  EXPECT_EQ(k1.operator()(), k2.operator()());
}

template <typename ViewT>
static void compare1d(ViewT const& k1, ViewT const& k2) {
  compareBasic(k1,k2);
  compareInner1d<ViewT,1>(k1,k2);
}

template <typename ViewT>
static void compare2d(ViewT const& k1, ViewT const& k2) {
  compareBasic(k1,k2);
  compareInner2d<ViewT,1>(k1,k2);
}

template <typename ViewT>
static void compare3d(ViewT const& k1, ViewT const& k2) {
  compareBasic(k1,k2);
  compareInner3d<ViewT,1>(k1,k2);
}

// N-D dimension comparison
template <typename ViewT>
static void compareND(ViewT const& k1, ViewT const& k2) {
  serdes::ViewEquality<ViewT>::template compare<GTestEquality>(k1,k2);
}

template <typename ParamT>
struct KokkosViewTest : ::testing::TestWithParam<ParamT> {
  virtual void SetUp() override {
    Kokkos::initialize();
  }
  virtual void TearDown() override {
    Kokkos::finalize();
  }
};

template <typename ParamT> struct KokkosViewTest1D : KokkosViewTest<ParamT> { };
template <typename ParamT> struct KokkosViewTest2D : KokkosViewTest<ParamT> { };
template <typename ParamT> struct KokkosViewTest3D : KokkosViewTest<ParamT> { };

// 0-D initialization
template <typename T, typename... Args>
static inline void init0d(Kokkos::View<T,Args...> const& v) {
  v.operator()() = 29;
}

// 1-D initialization
template <typename T, typename... Args>
static inline void init1d(Kokkos::View<T*,Args...> const& v) {
  for (auto i = 0; i < v.extent(0); i++) {
    v.operator()(i) = i;
  }
}

template <typename T, unsigned N, typename... Args>
static inline void init1d(Kokkos::View<T[N],Args...> const& v) {
  EXPECT_EQ(N, v.extent(0));
  for (auto i = 0; i < v.extent(0); i++) {
    v.operator()(i) = i;
  }
}

template <typename T, typename... Args>
static inline void init1d(
  Kokkos::Experimental::DynamicView<T*,Args...> const& v
) {
  for (auto i = 0; i < v.extent(0); i++) {
    v.operator()(i) = i;
  }
}

// 2-D initialization
template <typename T, typename... Args>
static inline void init2d(Kokkos::View<T**,Args...> const& v) {
  for (auto i = 0; i < v.extent(0); i++) {
    for (auto j = 0; j < v.extent(1); j++) {
      v.operator()(i,j) = (i*v.extent(1))+j;
    }
  }
}

template <typename T, unsigned N, typename... Args>
static inline void init2d(Kokkos::View<T*[N],Args...> const& v) {
  EXPECT_EQ(N, v.extent(1));
  for (auto i = 0; i < v.extent(0); i++) {
    for (auto j = 0; j < N; j++) {
      v.operator()(i,j) = i*N+j;
    }
  }
}

// 3-D initialization
template <typename T, typename... Args>
static inline void init3d(Kokkos::View<T***,Args...> const& v) {
  for (auto i = 0; i < v.extent(0); i++) {
    for (auto j = 0; j < v.extent(1); j++) {
      for (auto k = 0; k < v.extent(2); k++) {
        v.operator()(i,j,k) = (i*v.extent(0)*v.extent(1))+(j*v.extent(0))+k;
      }
    }
  }
}

template <typename T, unsigned N, typename... Args>
static inline void init3d(Kokkos::View<T**[N],Args...> const& v) {
  for (auto i = 0; i < v.extent(0); i++) {
    for (auto j = 0; j < v.extent(1); j++) {
      for (auto k = 0; k < N; k++) {
        v.operator()(i,j,k) = (i*v.extent(0)*v.extent(1))+(j*v.extent(0))+k;
      }
    }
  }
}

using lsType = std::size_t;

template <typename LayoutT>
inline LayoutT layout1d(lsType d1) {
  return LayoutT{d1};
}

template <typename LayoutT>
inline LayoutT layout2d(lsType d1, lsType d2) {
  return LayoutT{d1,d2};
}

template <typename LayoutT>
inline LayoutT layout3d(lsType d1, lsType d2, lsType d3) {
  return LayoutT{d1,d2,d3};
}

template <typename LayoutT>
inline LayoutT layout4d(lsType d1, lsType d2, lsType d3, lsType d4) {
  return LayoutT{d1,d2,d3,d4};
}

// Be very careful here: Kokkos strides must be laid out properly for this to be
// correct. This computes a simple strided layout. Strides are absolute for each
// dimension and shall calculated as such.
template <>
inline Kokkos::LayoutStride layout1d(lsType d1) {
  return Kokkos::LayoutStride{d1,1};
}

template <>
inline Kokkos::LayoutStride layout2d(lsType d1,lsType d2) {
  return Kokkos::LayoutStride{d1,1,d2,d1};
}

template <>
inline Kokkos::LayoutStride layout3d(lsType d1,lsType d2,lsType d3) {
  return Kokkos::LayoutStride{d1,1,d2,d1,d3,d1*d2};
}

template <>
inline Kokkos::LayoutStride layout4d(lsType d1,lsType d2,lsType d3,lsType d4) {
  return Kokkos::LayoutStride{d1,1,d2,d1,d3,d1*d2,d4,d1*d2*d3};
}

TYPED_TEST_CASE_P(KokkosViewTest1D);
TYPED_TEST_CASE_P(KokkosViewTest2D);
TYPED_TEST_CASE_P(KokkosViewTest3D);

TYPED_TEST_P(KokkosViewTest1D, test_1d_any) {
  using namespace serialization::interface;

  using LayoutType = typename std::tuple_element<0,TypeParam>::type;
  using DataType   = typename std::tuple_element<1,TypeParam>::type;
  using ViewType = Kokkos::View<DataType, LayoutType>;

  static constexpr size_t const N = 241;

  LayoutType layout = layout1d<LayoutType>(N);
  ViewType in_view("test", layout);

  init1d(in_view);

  auto ret = serialize<ViewType>(in_view);
  auto out_view = deserialize<ViewType>(ret->getBuffer(), ret->getSize());
  auto const& out_view_ref = *out_view;

#if SERDES_USE_ND_COMPARE
  compareND(in_view, out_view_ref);
#else
  compare1d(in_view, out_view_ref);
#endif
}

TYPED_TEST_P(KokkosViewTest2D, test_2d_any) {
  using namespace serialization::interface;

  using LayoutType = typename std::tuple_element<0,TypeParam>::type;
  using DataType   = typename std::tuple_element<1,TypeParam>::type;
  using ViewType = Kokkos::View<DataType, LayoutType>;

  static constexpr size_t const N = 23;
  static constexpr size_t const M = 32;

  constexpr auto dyn = serdes::CountDims<ViewType,DataType>::dynamic;

  LayoutType layout = layout2d<LayoutType>(N,M);
  //dyn == 2 ? layout2d<LayoutType>(N,M) : layout1d<LayoutType>(N);
  ViewType in_view("test-2D-some-string", layout);

  init2d(in_view);

  auto ret = serialize<ViewType>(in_view);
  auto out_view = deserialize<ViewType>(ret->getBuffer(), ret->getSize());
  auto const& out_view_ref = *out_view;

#if SERDES_USE_ND_COMPARE
  compareND(in_view, out_view_ref);
#else
  compare2d(in_view, out_view_ref);
#endif
}

TYPED_TEST_P(KokkosViewTest3D, test_3d_any) {
  using namespace serialization::interface;

  using LayoutType = typename std::tuple_element<0,TypeParam>::type;
  using DataType   = typename std::tuple_element<1,TypeParam>::type;
  using ViewType = Kokkos::View<DataType, LayoutType>;

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

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest1D, test_1d_any);
REGISTER_TYPED_TEST_CASE_P(KokkosViewTest2D, test_2d_any);
REGISTER_TYPED_TEST_CASE_P(KokkosViewTest3D, test_3d_any);

// SpliceTypeMeta: zip a set of types with ValueT

template <int N, typename ValueT, typename TypeList>
struct SpliceTypeMeta {
  using ResultType = decltype(
    std::tuple_cat(
      std::declval<
        std::tuple<
          std::tuple<ValueT,typename std::tuple_element<N-1,TypeList>::type>
        >
      >(),
      std::declval<typename SpliceTypeMeta<N-1,ValueT,TypeList>::ResultType>()
    )
  );
};

template <typename ValueT, typename TypeList>
struct SpliceTypeMeta<0,ValueT,TypeList> {
  using ResultType = decltype(std::declval<std::tuple<>>());
};

template <typename TypeList, typename ValueT>
struct SpliceMeta {
  using ResultTupleType = typename SpliceTypeMeta<
    std::tuple_size<TypeList>::value,ValueT,TypeList
  >::ResultType;
};

//
// Debug code for SpliceTypeMeta
//
// struct A { A() = delete; };
// using TestA = std::tuple<int, float>;
// using TestB = std::tuple<std::tuple<A,float>, std::tuple<A,int>>;
// using TestC = typename SpliceMeta<TestA,A>::ResultTupleType;
// int static_test_of() { TestC{}; }
//
// static_assert(
//   std::is_same<TestB,TestC>::value, "Not same"
// );
//

// Type conversion from std::tuple<X,Y,Z> to testing::Types<X,Y,Z>
template <typename Tuple1, template <typename...> class Tuple2>
struct ConvertTupleType { };

template <template <typename...> class Tuple2, typename... Args>
struct ConvertTupleType<std::tuple<Args...>,Tuple2> {
  using ResultType = Tuple2<Args...>;
};

// Factory for constructed generated type param list for testing::Types
template <typename TypeList, typename ValueT>
struct TestFactory {
  static constexpr auto const tup_size = std::tuple_size<TypeList>::value;
  static constexpr auto const I = std::make_index_sequence<tup_size>{};
  using ResultTupleType = typename SpliceMeta<TypeList,ValueT>::ResultTupleType;
  using ResultType =
    typename ConvertTupleType<ResultTupleType,testing::Types>::ResultType;
};

#if DO_UNIT_TESTS_FOR_VIEW

///////////////////////////////////////////////////////////////////////////////
// 1-D Kokkos::View Tests
///////////////////////////////////////////////////////////////////////////////

using Test1DTypes = std::tuple<
  int      *, int      [1], int      [4],
  double   *, double   [1], double   [4],
  float    *, float    [1], float    [4],
  int32_t  *, int32_t  [1], int32_t  [4],
  int64_t  *, int64_t  [1], int64_t  [4],
  unsigned *, unsigned [1], unsigned [4],
  long     *, long     [1], long     [4],
  long long*, long long[1], long long[4]
>;

using Test1DTypesLeft =
  typename TestFactory<Test1DTypes,Kokkos::LayoutLeft>::ResultType;
using Test1DTypesRight =
  typename TestFactory<Test1DTypes,Kokkos::LayoutRight>::ResultType;
using Test1DTypesStride =
  typename TestFactory<Test1DTypes,Kokkos::LayoutStride>::ResultType;

INSTANTIATE_TYPED_TEST_CASE_P(test_1d_L, KokkosViewTest1D, Test1DTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_1d_R, KokkosViewTest1D, Test1DTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_1d_S, KokkosViewTest1D, Test1DTypesStride);

///////////////////////////////////////////////////////////////////////////////
// 2-D Kokkos::View Tests
///////////////////////////////////////////////////////////////////////////////

using Test2DTypes = std::tuple<
  int      **, int      *[1], int      *[4],
  double   **, double   *[1], double   *[4],
  float    **, float    *[1], float    *[4],
  int32_t  **, int32_t  *[1], int32_t  *[4],
  int64_t  **, int64_t  *[1], int64_t  *[4],
  unsigned **, unsigned *[1], unsigned *[4],
  long     **, long     *[1], long     *[4],
  long long**, long long*[1], long long*[4]
>;

using Test2DTypesLeft =
  typename TestFactory<Test2DTypes,Kokkos::LayoutLeft>::ResultType;
using Test2DTypesRight =
  typename TestFactory<Test2DTypes,Kokkos::LayoutRight>::ResultType;
using Test2DTypesStride =
  typename TestFactory<Test2DTypes,Kokkos::LayoutStride>::ResultType;

INSTANTIATE_TYPED_TEST_CASE_P(test_2d_L, KokkosViewTest2D, Test2DTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_2d_R, KokkosViewTest2D, Test2DTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_2d_S, KokkosViewTest2D, Test2DTypesStride);


///////////////////////////////////////////////////////////////////////////////
// 3-D Kokkos::View Tests
///////////////////////////////////////////////////////////////////////////////

using Test3DTypes = std::tuple<
  int      ***, int      **[1], int      **[9],
  double   ***, double   **[1], double   **[9],
  float    ***, float    **[1], float    **[9],
  int32_t  ***, int32_t  **[1], int32_t  **[9],
  int64_t  ***, int64_t  **[1], int64_t  **[9],
  unsigned ***, unsigned **[1], unsigned **[9],
  long     ***, long     **[1], long     **[9],
  long long***, long long**[1], long long**[9]
>;

using Test3DTypesLeft =
  typename TestFactory<Test3DTypes,Kokkos::LayoutLeft>::ResultType;
using Test3DTypesRight =
  typename TestFactory<Test3DTypes,Kokkos::LayoutRight>::ResultType;
using Test3DTypesStride =
  typename TestFactory<Test3DTypes,Kokkos::LayoutStride>::ResultType;

INSTANTIATE_TYPED_TEST_CASE_P(test_3d_L, KokkosViewTest3D, Test3DTypesLeft);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_R, KokkosViewTest3D, Test3DTypesRight);
INSTANTIATE_TYPED_TEST_CASE_P(test_3d_S, KokkosViewTest3D, Test3DTypesStride);

#endif

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
  using Kokkos_ViewType1 = ::Kokkos::View<int*,     Kokkos::LayoutLeft>;
  using Kokkos_ViewType2 = ::Kokkos::View<double**, Kokkos::LayoutRight>;
  using Kokkos_ViewType3 = ::Kokkos::View<float***>;
  using Kokkos_ViewType4 = ::Kokkos::View<int*[2]>;
  using DimType          = typename Kokkos_ViewType1::size_type;

  Data() = default;

  explicit Data(DataConsTagType)
    : val1(gold_val1), val2(gold_val2), vec({gold_val3,gold_val3+1,gold_val3+2})
  {
    Kokkos_ViewType1 v1_tmp("v1_tmp",d1_a);
    Kokkos_ViewType2 v2_tmp("v2_tmp",d2_a,d2_b);
    Kokkos_ViewType3 v3_tmp("v3_tmp",d3_a,d3_b,d3_c);
    Kokkos_ViewType4 v4_tmp("v4_tmp",d4_a);

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
    v1 = v1_tmp;
    v2 = v2_tmp;
    v3 = v3_tmp;
    v4 = v4_tmp;
  }

  /* Generators for creating expected data values */
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
    EXPECT_EQ(in.v1.size(),d1_a);
    EXPECT_EQ(in.v2.size(),d2_a*d2_b);
    EXPECT_EQ(in.v3.size(),d3_a*d3_b*d3_c);
    EXPECT_EQ(in.v4.size(),d4_a*2);

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
  s | data.v1 | data.v2 | data.v3 | data.v4;
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

  //std::cout << "size=" << ret->getSize() << std::endl;

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

