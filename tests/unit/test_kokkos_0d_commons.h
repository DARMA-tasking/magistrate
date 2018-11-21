#ifndef TEST_KOKKOS_2D_COMMONS_H
#define TEST_KOKKOS_2D_COMMONS_H

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

#endif // TEST_KOKKOS_2D_COMMONS_H
