#ifndef TEST_KOKKOS_0D_COMMONS_H
#define TEST_KOKKOS_0D_COMMONS_H

#include "test_commons.h"

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

#if DO_UNIT_TESTS_FOR_VIEW

///////////////////////////////////////////////////////////////////////////////
// 0-D Kokkos::View Tests (a single value)
///////////////////////////////////////////////////////////////////////////////

using Test0DTypes = testing::Types<
  int      ,
  double   ,
  float    ,
  int32_t  ,
  int64_t  ,
  unsigned ,
  long     ,
  long long
>;

#endif

#endif // TEST_KOKKOS_0D_COMMONS_H
