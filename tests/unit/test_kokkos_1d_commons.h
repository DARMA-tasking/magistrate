#ifndef TEST_KOKKOS_1D_COMMONS_H
#define TEST_KOKKOS_1D_COMMONS_H

template <typename ViewT, unsigned ndim>
static void compareInner1d(ViewT const& k1, ViewT const& k2) {
  std::cout << "compareInner1d: " << k1.extent(0) << "," << k2.extent(0) << "\n";
  EXPECT_EQ(k1.extent(0), k2.extent(0));
  for (typename ViewT::size_type i = 0; i < k1.extent(0); i++) {
    EXPECT_EQ(k1.operator()(i), k2.operator()(i));
  }
}

template <typename ViewT>
static void compare1d(ViewT const& k1, ViewT const& k2) {
  compareBasic(k1,k2);
  compareInner1d<ViewT,1>(k1,k2);
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


template <typename LayoutT>
inline LayoutT layout1d(lsType d1) {
  return LayoutT{d1};
}

// Be very careful here: Kokkos strides must be laid out properly for this to be
// correct. This computes a simple strided layout. Strides are absolute for each
// dimension and shall calculated as such.
template <>
inline Kokkos::LayoutStride layout1d(lsType d1) {
  return Kokkos::LayoutStride{d1,1};
}
#endif // TEST_KOKKOS_1D_COMMONS_H
