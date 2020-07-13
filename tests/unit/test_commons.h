/*
//@HEADER
// *****************************************************************************
//
//                                test_commons.h
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
#ifndef TEST_COMMONS_H
#define TEST_COMMONS_H

#include "test_harness.h"

#include <checkpoint/checkpoint.h>
#include <checkpoint/container/view_equality.h>

#if KOKKOS_ENABLED_CHECKPOINT

#include <Kokkos_Core.hpp>
#include <Kokkos_View.hpp>
#include <Kokkos_DynamicView.hpp>
#include <Kokkos_Serial.hpp>

#include <functional>

/*
 * Compiling all the unit tests for Kokkos::View takes a long time, thus a
 * compile-time option to disable the unit tests if needed
 */
#define DO_UNIT_TESTS_FOR_VIEW 1

#endif

// By default, using manual compare...should I switch this?
#define CHECKPOINT_USE_ND_COMPARE 0

/*
 * This manual compare code should be removed once checkpoint::ViewEquality is fully
 * tested on target platforms
 */

struct GTestEquality {
  template <typename T>
  bool operator()(T&& a, T&& b) const {
    EXPECT_EQ(a,b);
    return a == b;
  }
};

template <typename ViewT>
static void compareBasic(ViewT const& k1, ViewT const& k2) {
  using EqualityType = checkpoint::ViewEquality<ViewT>;
  EqualityType::template compareStaticDim<GTestEquality>(k1);
  EqualityType::template compareStaticDim<GTestEquality>(k2);
  EqualityType::template compareMeta<GTestEquality>(k1,k2);
}

#if KOKKOS_ENABLED_CHECKPOINT
template <typename ParamT>
struct KokkosViewTest : ::testing::TestWithParam<ParamT> { };
#endif

using lsType = std::size_t;

// SpliceTypeMeta: zip a set of types with ValueT

template <int N, typename ValueT, typename TypeList>
struct SpliceTypeMeta {
  using ResultType = decltype(
    std::tuple_cat(
      std::declval<
        std::tuple<
          std::tuple<typename std::tuple_element<N-1,TypeList>::type,ValueT>
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


namespace  {
template <typename T>
std::unique_ptr<T> serializeAny(
  T& view, std::function<void(T const&,T const&)> compare
) {
  using namespace checkpoint;

  auto ret = serialize<T>(view);
  auto out_view = deserialize<T>(ret->getBuffer());
  auto const& out_view_ref = *out_view;
  #if CHECKPOINT_USE_ND_COMPARE
    compareND(view, out_view_ref);
  #else
    compare(view, out_view_ref);
  #endif
  return std::move(out_view);
}
} //end namespace

#endif // TEST_COMMONS_H
