/*
//@HEADER
// *****************************************************************************
//
//                           test_kokkos_1d_commons.h
//                           DARMA Toolkit v. 1.0.0
//                 DARMA/checkpoint => Serialization Library
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
#ifndef TEST_KOKKOS_1D_COMMONS_H
#define TEST_KOKKOS_1D_COMMONS_H

#include "test_commons.h"

#include <Kokkos_DynRankView.hpp>

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
  for (auto i = 0UL; i < v.extent(0); i++) {
    v.operator()(i) = i;
  }
}

template <typename T, typename... Args>
static inline void init1d(Kokkos::DynRankView<T,Args...> const& v) {
  for (auto i = 0UL; i < v.extent(0); i++) {
    v.operator()(i) = i;
  }
}

template <typename T, unsigned N, typename... Args>
static inline void init1d(Kokkos::View<T[N],Args...> const& v) {
  EXPECT_EQ(N, v.extent(0));
  for (auto i = 0UL; i < v.extent(0); i++) {
    v.operator()(i) = i;
  }
}

template <typename T, typename... Args>
static inline void init1d(
  Kokkos::Experimental::DynamicView<T*,Args...> const& v
) {
  for (auto i = 0UL; i < v.extent(0); i++) {
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

#if DO_UNIT_TESTS_FOR_VIEW

///////////////////////////////////////////////////////////////////////////////
// 1-D Kokkos::View Tests
///////////////////////////////////////////////////////////////////////////////

using Test1DTypes = std::tuple<
  int      *, int      [1], int      [3],
  double   *, double   [1], double   [4],
  float    *, float    [1], float    [4],
  int32_t  *, int32_t  [1], int32_t  [4],
  int64_t  *, int64_t  [1], int64_t  [4],
  unsigned *, unsigned [1], unsigned [4],
  long     *, long     [1], long     [4],
  long long*, long long[1], long long[4]
>;

using Test1DConstTypes = std::tuple<
  int       const *, int       const [1], int       const [3],
  double    const *, double    const [1], double    const [4],
  float     const *, float     const [1], float     const [4],
  int32_t   const *, int32_t   const [1], int32_t   const [4],
  int64_t   const *, int64_t   const [1], int64_t   const [4],
  unsigned  const *, unsigned  const [1], unsigned  const [4],
  long      const *, long      const [1], long      const [4],
  long long const *, long long const [1], long long const [4]
>;

using Test1DTypesLeft =
  typename TestFactory<Test1DTypes,Kokkos::LayoutLeft>::ResultType;
using Test1DTypesRight =
  typename TestFactory<Test1DTypes,Kokkos::LayoutRight>::ResultType;
using Test1DTypesStride =
  typename TestFactory<Test1DTypes,Kokkos::LayoutStride>::ResultType;

using Test1DConstTypesLeft =
  typename TestFactory<Test1DConstTypes,Kokkos::LayoutLeft>::ResultType;
using Test1DConstTypesRight =
  typename TestFactory<Test1DConstTypes,Kokkos::LayoutRight>::ResultType;
using Test1DConstTypesStride =
  typename TestFactory<Test1DConstTypes,Kokkos::LayoutStride>::ResultType;

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

using DynRankViewTestTypes = testing::Types<
  int,
  double,
  float,
  int32_t,
  int64_t,
  unsigned,
  long,
  long long
>;

#endif

#endif // TEST_KOKKOS_1D_COMMONS_H
