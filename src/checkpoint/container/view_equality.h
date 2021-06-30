/*
//@HEADER
// *****************************************************************************
//
//                               view_equality.h
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

#if !defined INCLUDED_CHECKPOINT_CONTAINER_VIEW_EQUALITY_H
#define INCLUDED_CHECKPOINT_CONTAINER_VIEW_EQUALITY_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/serializers_headers.h"
#include "checkpoint/container/view_traits_extract.h"
#include "checkpoint/container/view_traverse_manual.h"
#include "checkpoint/container/view_traverse_ndim.h"

#if KOKKOS_ENABLED_CHECKPOINT

#include <Kokkos_Core.hpp>
#include <Kokkos_View.hpp>
#include <Kokkos_DynamicView.hpp>
#include <Kokkos_Serial.hpp>
#include <Kokkos_DynRankView.hpp>

#include <array>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>
#include <tuple>
#include <type_traits>
#include <functional>
#include <iostream>

/*
 * Macro used for convenience to apply eq operator, obtain return result, and
 * then return false if the eq operator returns false. This enables ViewEquality
 * to terminate when it find inequality deep in the stack.
 */
#define CHECKPOINT_APPLY_OP(eq, opA, opB, reveal)                           \
  do {                                                                  \
    if (!eq(opA,opB)) {                                                 \
      std::cout << "Failure on equal op: "                              \
                << "\n\t info=(" << #reveal << ", " << reveal << ")"    \
                << "\n\t opA=("  << #opA    << ", " << opA    << ")"    \
                << "\n\t !="                                            \
                << "\n\t opB=("  << #opB    << ", " << opB    << ")\n"; \
      return false;                                                     \
    }                                                                   \
  } while (0);

/*
 * Implement a super simple std::tuple<...> printer for the above overload
 * when the index as a std::tuple<...> is passed to reveal. This allows the
 * above function to print out the index that fails the quality test:
 */

namespace std {

template <typename Ch, typename Tr, typename Tuple, std::size_t... Is>
void printTuple(
  std::basic_ostream<Ch,Tr>& os, Tuple const& t, std::index_sequence<Is...>
) {
  using UnFoldType = int[];
  (void)UnFoldType{((os << (Is == 0 ? "" : ", ") << std::get<Is>(t)), 0)...};
}

template <typename Ch, typename Tr, typename... Args>
std::basic_ostream<Ch, Tr>& operator<<(
  std::basic_ostream<Ch, Tr>& os, std::tuple<Args...> const& t
) {
  constexpr auto size = std::tuple_size<std::tuple<Args...>>::value;
  os << "(";
  printTuple(os, t, std::make_index_sequence<size>{});
  return os << ")";
}

} /* end namespace std */

namespace checkpoint { namespace detail {

/*
 * Impl class in detail for comparing static to dynamic extents on a single view
 */
template <typename ViewT, typename Callable>
struct ViewEqualityStatic {

  // Compare the static dimension to the dynamic extent: it must always be
  // equal for correctness
  template <typename T, unsigned N, unsigned M, typename... Args>
  bool operator()(Kokkos::View<T*[N][M],Args...> const& v, Callable eq) {
    using ExtentType = decltype(v.extent(1));
    CHECKPOINT_APPLY_OP(eq, v.extent(1), static_cast<ExtentType>(N), N);
    CHECKPOINT_APPLY_OP(eq, v.extent(2), static_cast<ExtentType>(M), M);
    return true;
  }

  template <typename T, unsigned N, unsigned M, typename... Args>
  bool operator()(Kokkos::View<T[N][M],Args...> const& v, Callable eq) {
    using ExtentType = decltype(v.extent(1));
    CHECKPOINT_APPLY_OP(eq, v.extent(0), static_cast<ExtentType>(N), N);
    CHECKPOINT_APPLY_OP(eq, v.extent(1), static_cast<ExtentType>(M), M);
    return true;
  }

  template <typename T, unsigned N, typename... Args>
  bool operator()(Kokkos::View<T**[N],Args...> const& v, Callable eq) {
    using ExtentType = decltype(v.extent(1));
    CHECKPOINT_APPLY_OP(eq, v.extent(2), static_cast<ExtentType>(N), N);
    return true;
  }

  template <typename T, unsigned N, typename... Args>
  bool operator()(Kokkos::View<T*[N],Args...> const& v, Callable eq) {
    using ExtentType = decltype(v.extent(1));
    CHECKPOINT_APPLY_OP(eq, v.extent(1), static_cast<ExtentType>(N), N);
    return true;
  }

  template <typename T, unsigned N, typename... Args>
  bool operator()(Kokkos::View<T[N],Args...> const& v, Callable eq) {
    using ExtentType = decltype(v.extent(1));
    CHECKPOINT_APPLY_OP(eq, v.extent(0), static_cast<ExtentType>(N), N);
    return true;
  }

  template <typename AnyT, typename... Args>
  bool operator()(Kokkos::View<AnyT,Args...> const& v, Callable eq) {
    // No static dimension to check against dynamic dimension, return true
    return true;
  }

  template <typename AnyT, typename... Args>
  bool operator()(
    Kokkos::Experimental::DynamicView<AnyT,Args...> const& v, Callable eq
  ) {
    // No static dimension to check against dynamic dimension, return true
    return true;
  }

  template <typename AnyT, typename... Args>
  bool operator()(Kokkos::DynRankView<AnyT,Args...> const& v, Callable eq) {
    // No static dimension to check against dynamic dimension, return true
    return true;
  }

};

struct DefaultEQ {
  template <typename T>
  bool operator()(T&& a, T&& b) const {
    return a == b;
  }
};

}} /* end namespace checkpoint::detail */

namespace checkpoint {

/*
 *==============================================================================
 *
 *        Check equality of two views: meta-data, internal data, etc.
 *
 *
 *   ========== Example w/default equality operator: =================
 *
 *     checkpoint::ViewEquality<ViewT>::compare(view1,view2);
 *
 *   ========== Example w/user-specified equality operator: ==========
 *
 *     struct EqFunctor {
 *       template <typename T>
 *       bool operator()(T&& a, T&& b) const {
 *         return a == b;
 *       }
 *     };
 *
 *     checkpoint::ViewEquality<ViewT>::template compare<EqFunctor>(view1,view2);
 *
 *  Note: if you specify your own equality functor, it needs to handle l-values
 *  and r-values based on how ViewEquality uses it (so use a universal reference
 *  as above or one may write multiple overloads for each type of
 *  reference). You can also specify a lambda or even two lambdas, one typed
 *  with the underlying data type of the view.
 *
 *  Depending on what your equality functor does, the comparison code will not
 *  stop when (in)equality is determined unless the functor returns false.
 *
 *==============================================================================
 */

template <typename ViewT, typename ViewU = ViewT>
struct ViewEquality {
  using DataType     = typename checkpoint::ViewGetType<ViewT>::DataType;
  using CountDimType = checkpoint::CountDims<ViewT>;
  using BaseType     = typename CountDimType::BaseT;

  /*
   * Static check on the layout of the two views in question
   */
  using LayoutT = typename ViewT::array_layout;
  using LayoutU = typename ViewU::array_layout;
  // Do the static check right here: should break in instantiation if ViewT and
  // ViewU do not have the same layout
  static_assert(std::is_same<LayoutT, LayoutU>::value, "Must be same layout");

  /*
   * This is the main compare call: it compares between two views the statically
   * specified extents in the type wrt to the dyanmic extents, extent equality
   * across two views, the meta-data in the view (label,span,etc.), and the
   * actual internal data inside the views by invoking the quality functor
   */
  template <typename DataEq = detail::DefaultEQ, typename MetaEq = DataEq>
  static bool compare(
    ViewT const& v1, ViewU const& v2, DataEq eqd = {}, MetaEq eqm = {}
  ) {
    // Check static dimension wrt to dynamic extents for both views
    auto const static_v1_ret   = compareStaticDim(v1, eqm);
    if (!static_v1_ret)   { return false; }
    auto const static_v2_ret   = compareStaticDim(v2, eqm);
    if (!static_v2_ret)   { return false; }
    // Compare the extents of v1 and v2
    auto const extent_test_ret = compareExtents(v1, v2, eqm);
    if (!extent_test_ret) { return false; }
    // Compare the meta-data for v1 and v2
    auto const meta_test_ret   = compareMeta(v1, v2, eqm);
    if (!meta_test_ret)   { return false; }
    // Compare the actual data in v1 and v2
    auto const data_test_ret   = compareData(v1, v2, eqd);
    if (!data_test_ret)   { return false; }
    return true;
  }

  /*
   * This compares the equality for a single view of the static type-specified
   * extents to the dynamic extents to ensure equality
   */
  template <typename EqT = detail::DefaultEQ>
  static bool compareStaticDim(ViewT const& v1, EqT c = {}) {
    detail::ViewEqualityStatic<ViewT,EqT> static_eq;
    return static_eq.operator()(v1,c);
  }

  /*
   * This compares the extents of two views, including the static and dynamic
   * extents
   */
  template <typename EqT = detail::DefaultEQ>
  static bool compareExtents(ViewT const& v1, ViewU const& v2, EqT eq = {}) {
    auto const ndims_v1 = CountDims<ViewT>::numDims(v1);
    auto const ndims_v2 = CountDims<ViewU>::numDims(v2);
    CHECKPOINT_APPLY_OP(eq, ndims_v1, ndims_v2, "num dims equal");
    for (auto i = 0; i < ndims_v1; i++) {
      CHECKPOINT_APPLY_OP(eq, v1.extent(i), v2.extent(i), i);
    }
    return true;
  }

  /*
   * This compares the view meta-data: size, label, contig, span
   */
  template <typename EqT = detail::DefaultEQ>
  static bool compareMeta(ViewT const& v1, ViewU const& v2, EqT eq = {}) {
    CHECKPOINT_APPLY_OP(eq, v1.label(),              v2.label(), "label");
    CHECKPOINT_APPLY_OP(eq, v1.size(),               v2.size(), "size");
    CHECKPOINT_APPLY_OP(eq, v1.span_is_contiguous(), v2.span_is_contiguous(), "contig");
    CHECKPOINT_APPLY_OP(eq, v1.span(),               v2.span(), "span");
    return true;
  }

  /*
   * This compares the N-D internal data between two inputs that may be a
   * Kokkos::View<T> or Kokkos::DynamicView<T> (or conform to the interface). It
   * recursively traverses both of them and applies a supplied operator by
   * essentially zipping the elements
   */
  template <typename EqT = detail::DefaultEQ>
  static bool compareData(ViewT const& v1, ViewU const& v2, EqT eq) {
    using checkpoint::TraverseRecursive;

    using TupleType     = std::tuple<ViewT,ViewT>;

    constexpr auto dims = CountDimType::dynamic;

    using RecurType     = TraverseRecursive<TupleType,DataType,dims>;
    using IndexType     = typename RecurType::IndexType;

    // Extract the index type above with the default (phony) op to extract the
    // IndexType, which is needed below without using "auto" for lambda
    // parameters, which I am trying to avoid.
    auto fn = [eq](IndexType index, BaseType& elm1, BaseType& elm2){
      auto const is_equal = eq(elm1,elm2);
      if (!is_equal) {
        CHECKPOINT_APPLY_OP(eq, elm1, elm2, index);
      }
      return is_equal;
    };

    // Returns the number of comparisons in the inner data that returned false
    using FnT = decltype(fn);
    auto const neq = TraverseRecursive<TupleType,DataType,dims,FnT>::apply(
      std::make_tuple(v1,v2),fn
    );

    return neq == 0;
  }

};

} /* end namespace checkpoint */

#endif /*KOKKOS_ENABLED_CHECKPOINT*/

#endif /*INCLUDED_CHECKPOINT_CONTAINER_VIEW_EQUALITY_H*/
