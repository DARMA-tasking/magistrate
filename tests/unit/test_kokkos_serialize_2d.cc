#if KOKKOS_ENABLED_SERDES

#include "test_harness.h"
#include "test_commons.h"

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

template <typename ViewT>
static void compare2d(ViewT const& k1, ViewT const& k2) {
  compareBasic(k1,k2);
  compareInner2d<ViewT,1>(k1,k2);
}

template <typename ParamT> struct KokkosViewTest2D : KokkosViewTest<ParamT> { };

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

template <typename LayoutT>
inline LayoutT layout2d(lsType d1, lsType d2) {
  return LayoutT{d1,d2};
}

// Be very careful here: Kokkos strides must be laid out properly for this to be
// correct. This computes a simple strided layout. Strides are absolute for each
// dimension and shall calculated as such.
template <>
inline Kokkos::LayoutStride layout2d(lsType d1,lsType d2) {
  return Kokkos::LayoutStride{d1,1,d2,d1};
}

TYPED_TEST_CASE_P(KokkosViewTest2D);

TYPED_TEST_P(KokkosViewTest2D, test_2d_any) {
  using namespace serialization::interface;

  using LayoutType = typename std::tuple_element<0,TypeParam>::type;
  using DataType   = typename std::tuple_element<1,TypeParam>::type;
  using ViewType   = Kokkos::View<DataType, LayoutType>;

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

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest2D, test_2d_any);

#if DO_UNIT_TESTS_FOR_VIEW

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

#endif
#endif