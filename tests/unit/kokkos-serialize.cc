#if KOKKOS_ENABLED_SERDES

#include "test_harness.h"
#include <Kokkos_Core.hpp>
#include <Kokkos_View.hpp>
#include <Kokkos_Serial.hpp>
#include <container/array_serialize.h>
#include <container/view_serialize.h>
#include <container/string_serialize.h>
#include <container/vector_serialize.h>
#include <container/tuple_serialize.h>
#include <serializers/packer.h>
#include <serializers/unpacker.h>
#include <serdes_headers.h>

#include "serdes_headers.h"
#include "serialization_library_headers.h"

template <typename ViewTypeA, typename ViewTypeB>
static void isSameMemoryLayout(ViewTypeA const&, ViewTypeB const&) {
  using array_layoutA = typename ViewTypeA::array_layout;
  using array_layoutB = typename ViewTypeB::array_layout;
  static_assert(
    std::is_same<array_layoutA, array_layoutB>::value, "Must be same layout"
  );
}

template <typename ViewT, unsigned ndim>
static void compareInner1d(ViewT const& k1, ViewT const& k2) {
  EXPECT_EQ(k1.extent(0), k2.extent(0));
  for (typename ViewT::size_type i = 0; i < k1.extent(0); i++) {
    // std::cout << "i=" << i << ", val1=" << k1.operator()(i) << "" << ", val2=" << k2.operator()(i) <<"\n";
    EXPECT_EQ(k1.operator()(i), k2.operator()(i));
    // EXPECT_EQ(k1.operator()(i,1), k2.operator()(i,1));
  }
}

template <typename ViewT, unsigned ndim>
static void compareInner2d(ViewT const& k1, ViewT const& k2) {
  std::cout << "compareInner2d: " << k1.extent(0) << "," << k2.extent(0) << std::endl;
  std::cout << "compareInner2d: " << k1.extent(1) << "," << k2.extent(1) << std::endl;
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
  std::cout << "compareInner3d: " << k1.extent(0) << "," << k2.extent(0) << std::endl;
  std::cout << "compareInner3d: " << k1.extent(1) << "," << k2.extent(1) << std::endl;
  std::cout << "compareInner3d: " << k1.extent(2) << "," << k2.extent(2) << std::endl;
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

template <typename ViewT>
static void compare1d(ViewT const& k1, ViewT const& k2) {
  EXPECT_EQ(k1.label(),         k2.label());
  EXPECT_EQ(k1.size(),          k2.size());
  EXPECT_EQ(k1.is_contiguous(), k2.is_contiguous());
  EXPECT_EQ(k1.use_count(),     k2.use_count());
  EXPECT_EQ(k1.span(),          k2.span());
  isSameMemoryLayout(k1, k2);
  compareInner1d<ViewT,1>(k1,k2);
}

template <typename ViewT>
static void compare2d(ViewT const& k1, ViewT const& k2) {
  EXPECT_EQ(k1.label(),         k2.label());
  EXPECT_EQ(k1.size(),          k2.size());
  EXPECT_EQ(k1.is_contiguous(), k2.is_contiguous());
  EXPECT_EQ(k1.use_count(),     k2.use_count());
  EXPECT_EQ(k1.span(),          k2.span());
  isSameMemoryLayout(k1, k2);
  compareInner2d<ViewT,1>(k1,k2);
}

template <typename ViewT>
static void compare3d(ViewT const& k1, ViewT const& k2) {
  EXPECT_EQ(k1.label(),         k2.label());
  EXPECT_EQ(k1.size(),          k2.size());
  EXPECT_EQ(k1.is_contiguous(), k2.is_contiguous());
  EXPECT_EQ(k1.use_count(),     k2.use_count());
  EXPECT_EQ(k1.span(),          k2.span());
  isSameMemoryLayout(k1, k2);
  compareInner3d<ViewT,1>(k1,k2);
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
  std::cout << "doing this\n";
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

template <>
inline Kokkos::LayoutStride layout1d(lsType d1) {
  return Kokkos::LayoutStride{d1,1};
}

template <>
inline Kokkos::LayoutStride layout2d(lsType d1,lsType d2) {
  std::cout << "USING OVERLOAD for layout2d\n";
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

  compare1d(in_view, out_view_ref);
}

TYPED_TEST_P(KokkosViewTest2D, test_2d_any) {
  using namespace serialization::interface;

  using LayoutType = typename std::tuple_element<0,TypeParam>::type;
  using DataType   = typename std::tuple_element<1,TypeParam>::type;
  using ViewType = Kokkos::View<DataType, LayoutType>;

  static constexpr size_t const N = 23;
  static constexpr size_t const M = 32;

  constexpr auto dyn =
    serdes::detail::Helper<void,ViewType,DataType>::dynamic_count;

  LayoutType layout = layout2d<LayoutType>(N,M);
  //dyn == 2 ? layout2d<LayoutType>(N,M) : layout1d<LayoutType>(N);
  ViewType in_view("test-2D-some-string", layout);

  init2d(in_view);

  auto ret = serialize<ViewType>(in_view);
  auto out_view = deserialize<ViewType>(ret->getBuffer(), ret->getSize());
  auto const& out_view_ref = *out_view;

  compare2d(in_view, out_view_ref);
}

TYPED_TEST_P(KokkosViewTest3D, test_3d_any) {
  using namespace serialization::interface;

  using LayoutType = typename std::tuple_element<0,TypeParam>::type;
  using DataType   = typename std::tuple_element<1,TypeParam>::type;
  using ViewType = Kokkos::View<DataType, LayoutType>;

  static constexpr size_t const N = 5;
  static constexpr size_t const M = 17;
  static constexpr size_t const Q = 7;

  constexpr auto dyn =
    serdes::detail::Helper<void,ViewType,DataType>::dynamic_count;

  LayoutType layout = layout3d<LayoutType>(N,M,Q);
  ViewType in_view("test-2D-some-string", layout);

  init3d(in_view);

  auto ret = serialize<ViewType>(in_view);
  auto out_view = deserialize<ViewType>(ret->getBuffer(), ret->getSize());
  auto const& out_view_ref = *out_view;

  compare3d(in_view, out_view_ref);
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

