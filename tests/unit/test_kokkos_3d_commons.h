#ifndef TEST_KOKKOS_3D_COMMONS_H
#define TEST_KOKKOS_3D_COMMONS_H

#include "test_harness.h"
#include "test_commons.h"

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

template <typename ViewT>
static void compare3d(ViewT const& k1, ViewT const& k2) {
  compareBasic(k1,k2);
  compareInner3d<ViewT,1>(k1,k2);
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

template <typename LayoutT>
inline LayoutT layout3d(lsType d1, lsType d2, lsType d3) {
  return LayoutT{d1,d2,d3};
}

// Be very careful here: Kokkos strides must be laid out properly for this to be
// correct. This computes a simple strided layout. Strides are absolute for each
// dimension and shall calculated as such.
template <>
inline Kokkos::LayoutStride layout3d(lsType d1,lsType d2,lsType d3) {
  return Kokkos::LayoutStride{d1,1,d2,d1,d3,d1*d2};
}

template <typename ViewT>
static void compare0d(ViewT const& k1, ViewT const& k2) {
  compareBasic(k1,k2);
  EXPECT_EQ(k1.operator()(), k2.operator()());
}

// 0-D initialization
template <typename T, typename... Args>
static inline void init0d(Kokkos::View<T,Args...> const& v) {
  v.operator()() = 29;
}

#endif // TEST_KOKKOS_3D_COMMONS_H
