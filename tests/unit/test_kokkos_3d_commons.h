/*
//@HEADER
// *****************************************************************************
//
//                           test_kokkos_3d_commons.h
//                 DARMA/magistrate => Serialization Library
//
// Copyright 2019 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// *****************************************************************************
//@HEADER
*/

#if !defined INCLUDED_TESTS_UNIT_TEST_KOKKOS_3D_COMMONS_H
#define INCLUDED_TESTS_UNIT_TEST_KOKKOS_3D_COMMONS_H

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
  for (auto i = 0UL; i < v.extent(0); i++) {
    for (auto j = 0UL; j < v.extent(1); j++) {
      for (auto k = 0UL; k < v.extent(2); k++) {
        v.operator()(i,j,k) = (i*v.extent(0)*v.extent(1))+(j*v.extent(0))+k;
      }
    }
  }
}

template <typename T, typename... Args>
static inline void init3d(Kokkos::DynRankView<T,Args...> const& v) {
  for (auto i = 0UL; i < v.extent(0); i++) {
    for (auto j = 0UL; j < v.extent(1); j++) {
      for (auto k = 0UL; k < v.extent(2); k++) {
        v.operator()(i,j,k) = (i*v.extent(0)*v.extent(1))+(j*v.extent(0))+k;
      }
    }
  }
}

template <typename T, unsigned N, typename... Args>
static inline void init3d(Kokkos::View<T**[N],Args...> const& v) {
  for (auto i = 0UL; i < v.extent(0); i++) {
    for (auto j = 0UL; j < v.extent(1); j++) {
      for (auto k = 0U; k < N; k++) {
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

#if DO_UNIT_TESTS_FOR_VIEW

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

using Test3DConstTypes = std::tuple<
  int       const ***, int       const **[1], int       const **[9],
  double    const ***, double    const **[1], double    const **[9],
  float     const ***, float     const **[1], float     const **[9],
  int32_t   const ***, int32_t   const **[1], int32_t   const **[9],
  int64_t   const ***, int64_t   const **[1], int64_t   const **[9],
  unsigned  const ***, unsigned  const **[1], unsigned  const **[9],
  long      const ***, long      const **[1], long      const **[9],
  long long const ***, long long const **[1], long long const **[9]
>;

using Test3DTypesLeft =
  typename TestFactory<Test3DTypes,Kokkos::LayoutLeft>::ResultType;
using Test3DTypesRight =
  typename TestFactory<Test3DTypes,Kokkos::LayoutRight>::ResultType;
using Test3DTypesStride =
  typename TestFactory<Test3DTypes,Kokkos::LayoutStride>::ResultType;

using Test3DConstTypesLeft =
  typename TestFactory<Test3DConstTypes,Kokkos::LayoutLeft>::ResultType;
using Test3DConstTypesRight =
  typename TestFactory<Test3DConstTypes,Kokkos::LayoutRight>::ResultType;
using Test3DConstTypesStride =
  typename TestFactory<Test3DConstTypes,Kokkos::LayoutStride>::ResultType;

#endif

template <typename ParamT> struct KokkosViewTest3D : KokkosViewTest<ParamT> { };

TYPED_TEST_CASE_P(KokkosViewTest3D);

TYPED_TEST_P(KokkosViewTest3D, test_3d_any) {
  using namespace checkpoint;

  using LayoutType = typename std::tuple_element<1,TypeParam>::type;
  using DataType   = typename std::tuple_element<0,TypeParam>::type;
  using ViewType          = Kokkos::View<DataType, LayoutType>;
  using NonConstT         = typename ViewType::traits::non_const_data_type;
  using NonConstViewType  = Kokkos::View<NonConstT, LayoutType>;
  using ConstT         = typename ViewType::traits::const_data_type;
  using ConstViewType  = Kokkos::View<ConstT, LayoutType>;
  static constexpr size_t const N = 5;
  static constexpr size_t const M = 17;
  static constexpr size_t const Q = 7;

  LayoutType layout = layout3d<LayoutType>(N,M,Q);
  NonConstViewType in_view("test-3D-some-string", layout);

  init3d(in_view);

  if (std::is_same<NonConstViewType, ViewType>::value) {
    serializeAny<NonConstViewType>(in_view, &compare3d<NonConstViewType>);
  } else {
    ConstViewType const_in_view = in_view;
    serializeAny<ConstViewType>(const_in_view, &compare3d<ConstViewType>);
  }
}

#endif /*INCLUDED_TESTS_UNIT_TEST_KOKKOS_3D_COMMONS_H*/
