/*
//@HEADER
// *****************************************************************************
//
//                                test_commons.h
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

#if !defined INCLUDED_UNIT_TEST_COMMONS_H
#define INCLUDED_UNIT_TEST_COMMONS_H

#include "test_harness.h"

#include <checkpoint/checkpoint.h>
#include <checkpoint/container/view_equality.h>

#if MAGISTRATE_KOKKOS_ENABLED

#include <Kokkos_Core.hpp>
#include <Kokkos_DynamicView.hpp>

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

#if MAGISTRATE_KOKKOS_ENABLED
template <typename ParamT>
struct KokkosViewTest : ::testing::TestWithParam<ParamT> { };
#endif

using lsType = std::size_t;

// SpliceTypeMeta: zip a set of types with ValueT

template <int N, typename ValueT, typename... List>
struct SpliceTypeMeta;

template <int N, typename ValueT, typename Type, typename... List>
struct SpliceTypeMeta<N, ValueT, Type, List...> {
  using ResultType = std::tuple<
    std::tuple<Type, ValueT>,
    typename std::tuple_element<
      0,
      typename SpliceTypeMeta<N-1, ValueT, List>::ResultType
    >::type...
  >;
};

template <typename ValueT, typename Type>
struct SpliceTypeMeta<1, ValueT, Type> {
  using ResultType = std::tuple<std::tuple<Type, ValueT>>;
};

template <typename ValueT, typename Tuple>
struct SpliceMeta;

template <typename ValueT, typename... Ts>
struct SpliceMeta<ValueT, std::tuple<Ts...>> {
  using ResultTupleType = typename SpliceTypeMeta<
    std::tuple_size<std::tuple<Ts...>>::value,
    ValueT,
    Ts...
  >::ResultType;
};

//
// Debug code for SpliceTypeMeta
//
// struct A { A() = delete; };
// using TestA = std::tuple<int, float>;
// using TestB = std::tuple<std::tuple<int,A>, std::tuple<float,A>>;
// using TestC = typename SpliceMeta<A, TestA>::ResultTupleType;
// //int static_test_of() { TestC{}; }

// static_assert(
//   std::is_same<TestB,TestC>::value, "Not same"
// );

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
  using ResultTupleType = typename SpliceMeta<ValueT, TypeList>::ResultTupleType;
  using ResultType =
    typename ConvertTupleType<ResultTupleType,testing::Types>::ResultType;
};


namespace  {
template <typename T>
std::unique_ptr<T> serializeAny(
  T& view, std::function<void(T const&,T const&)> compare = nullptr
) {
  using namespace checkpoint;

  auto ret = serialize<T>(view);
  auto out_view = deserialize<T>(ret->getBuffer());
  auto const& out_view_ref = *out_view;
  if (compare) {
    #if CHECKPOINT_USE_ND_COMPARE
      compareND(view, out_view_ref);
    #else
      compare(view, out_view_ref);
    #endif
  }
  return out_view;
}
} //end namespace

#endif /*INCLUDED_UNIT_TEST_COMMONS_H*/
