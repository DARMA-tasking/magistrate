/*
//@HEADER
// *****************************************************************************
//
//                             view_traverse_ndim.h
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

#if !defined INCLUDED_CHECKPOINT_CONTAINER_VIEW_TRAVERSE_NDIM_H
#define INCLUDED_CHECKPOINT_CONTAINER_VIEW_TRAVERSE_NDIM_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/serializers_headers.h"
#include "checkpoint/container/view_traits_extract.h"

#if KOKKOS_ENABLED_CHECKPOINT

#include <Kokkos_Core.hpp>
#include <Kokkos_View.hpp>
#include <Kokkos_DynamicView.hpp>
#include <Kokkos_Serial.hpp>

#include <utility>
#include <tuple>
#include <cassert>
#include <type_traits>
#include <cstdlib>

namespace checkpoint { namespace detail {

/*
 * Traversal of data backing a view by recursively traversing each dimension and
 * every element of the data one-by-one using the Kokkos::View::operator()(...)
 * or Kokkos::DynamicView::operator()(...). The access operator is guaranteed to
 * correctly accesses the data regardless of the layout. This new code does a
 * recursive traversal so it works for n-dimensional views.
 *
 * This code generally allows you to specify any callable, and use k-views if
 * you want to operate by essentially zipping the elements and passing to the
 * call function. You can compare equality, apply element-by-element equality,
 * or other n-dim operators on k-views.
 *
 */

// Recursive callable apply for n-dimension automatic traversal of a view
template <
  typename ViewT, typename T, typename TupleT, unsigned d, typename Callable
>
struct TraverseRecurImpl;

/*
 * Simple meta-code for detecting a tuple type
 */
template <typename T>
struct IsTuple : std::false_type {};
template <typename... U>
struct IsTuple<std::tuple<U...>> : std::true_type {};

/*
 * Extract the size_type from the View, overload to extract from
 * std::tuple<Kokkos::View...> of
 */
template <typename T>
struct ViewSizeTraits {
  using SizeType = typename T::size_type;
};

template <typename T, typename... Args>
struct ViewSizeTraits<std::tuple<T,Args...>> {
  using SizeType = typename T::size_type;
};

/*
 * General code for traversing a static or dynamic dimension and recursively
 * preceding pushing the index into the tuple
 */
template <
  typename ViewT, typename T, typename TupleT, unsigned d, typename Callable
>
struct TraverseRecurImplBase {
  using SizeType       = typename ViewSizeTraits<ViewT>::SizeType;
  template <typename U>
  using ViewIsTuple    = typename std::enable_if<IsTuple<U>::value,ViewT>::type;
  template <typename U>
  using ViewNotTuple   = typename std::enable_if<!IsTuple<U>::value,ViewT>::type;
  using CountType      = uint64_t;

  using TupleChildT    = decltype(
    std::tuple_cat(
      std::declval<TupleT>(),
      std::declval<std::tuple<SizeType>>()
    )
  );

  using ChildT         = TraverseRecurImpl<ViewT,T,TupleChildT,d-1,Callable>;
  using IndexTupleType = typename ChildT::IndexTupleType;

  template <typename U = ViewT>
  static CountType applyImpl(
    ViewT const& view, unsigned nd, TupleT idx, Callable call,
    ViewIsTuple<U>* x_ = nullptr
  ) {
    auto const ex1 = std::get<0>(view).extent(nd-d);
    auto const ex2 = std::get<1>(view).extent(nd-d);
    checkpointAssert(ex1 == ex2, "Matching extents must be equal");
    CountType neq = 0;
    for (SizeType i = 0; i < ex1; i++) {
      auto const ret = ChildT::apply(
        view,nd,std::tuple_cat(idx,std::tuple<SizeType>(i)),call
      );
      neq += !ret;
    }
    return neq;
  }

  template <typename U = ViewT>
  static CountType applyImpl(
    ViewT const& view, unsigned nd, TupleT idx, Callable call,
    ViewNotTuple<U>* x_ = nullptr
  ) {
    CountType neq = 0;
    for (SizeType i = 0; i < view.extent(nd-d); i++) {
      auto const ret = ChildT::apply(
        view,nd,std::tuple_cat(idx,std::tuple<SizeType>(i)),call
      );
      neq += !ret;
    }
    return neq;
  }

  static CountType apply(
    ViewT const& view, unsigned nd, TupleT idx, Callable call
  ) {
    return applyImpl<ViewT>(view,nd,idx,call);
  }
};

// Overload for traversing a static T[N] dimension
template <
  typename ViewT, typename T, unsigned N, typename TupleT, unsigned d,
  typename Callable
>
struct TraverseRecurImpl<ViewT, T[N], TupleT, d, Callable>
  : TraverseRecurImplBase<ViewT,T,TupleT,d,Callable>
{ };

// Overload for traversing a dynamic (T*) dimension
template <
  typename ViewT, typename T, typename TupleT, unsigned d, typename Callable
>
struct TraverseRecurImpl<ViewT, T*, TupleT, d, Callable>
  : TraverseRecurImplBase<ViewT,T,TupleT,d,Callable>
{ };

// Overload for traversing after we reach recursively the last dimension and now
// will dispatch the View::operator()()
template <
  typename ViewT, typename T, typename TupleT, unsigned d, typename Callable
>
struct TraverseRecurImpl {
  using SizeType     = typename ViewSizeTraits<ViewT>::SizeType;
  template <typename U>
  using ViewIsTuple  = typename std::enable_if<IsTuple<U>::value,ViewT>::type;
  template <typename U>
  using ViewNotTuple = typename std::enable_if<!IsTuple<U>::value,ViewT>::type;
  template <typename U>
  using GetBaseType  = typename CountDims<U>::BaseT;
  using IndexTupleType = TupleT;
  using CountType      = uint64_t;

  // Unwind the inner tuple for operator()(...)
  template <typename ViewU, std::size_t... I>
  static GetBaseType<ViewU>& expandTupleToOp(
    ViewU const& view, TupleT tup, std::index_sequence<I...> idx
  ) {
    return view.operator()(std::get<I>(tup)...);
  }

  // Unwind the inner tuple for operator()(...)
  template <typename ViewU>
  static GetBaseType<ViewU>& expandTupleToOp(ViewU const& view, TupleT tup) {
    constexpr auto size = std::tuple_size<TupleT>::value;
    return expandTupleToOp(view,tup,std::make_index_sequence<size>{});
  }

  // If it's a std::tuple<Kokkos::vie<T>...>, first unwind the outer tuple
  template <std::size_t... I>
  static bool dispatchViewTuple(
    ViewT const& view, Callable call, TupleT tup, std::index_sequence<I...>
  ) {
    // Dispatch the call operator on the view
    return call(tup,expandTupleToOp(std::get<I>(view),tup)...);
  }

  // Test whether the ViewT is actually a std::tuple<Kokkos::View<T>...>
  template <typename U = ViewT>
  static bool dispatchViewType(
    ViewT const& view, Callable call, TupleT tup, ViewIsTuple<U>* x_ = nullptr
  ) {
    constexpr auto size = std::tuple_size<ViewT>::value;
    return dispatchViewTuple(view,call,tup,std::make_index_sequence<size>{});
  }

  template <typename U = ViewT>
  static bool dispatchViewType(
    ViewT const& view, Callable call, TupleT tup, ViewNotTuple<U>* x_ = nullptr
  ) {
    call(expandTupleToOp(view,tup));
    return true;
  }

  static bool apply(ViewT const& view, unsigned, TupleT tup, Callable call) {
    return dispatchViewType(view,call,tup);
  }
};

struct DefaultRecurOP {
  template <typename T, typename U>
  bool operator()(U, T, T) const {
    checkpointAssert(false, "Wrong overload");
    return false; /* to avoid warning */
  }
};

}} /* end namespace checkpoint::detail */

namespace checkpoint {

template <
  typename ViewT, typename T, unsigned nd,
  typename Callable = detail::DefaultRecurOP
>
struct TraverseRecursive {
  using RecurT    = detail::TraverseRecurImpl<ViewT,T,std::tuple<>,nd,Callable>;
  using CountType = typename RecurT::CountType;
  using IndexType = typename RecurT::IndexTupleType;
  static CountType apply(ViewT const& view, Callable call) {
    return RecurT::apply(view,nd,std::tuple<>{},call);
  }
};

} /* end namespace checkpoint */

#endif /*KOKKOS_ENABLED_CHECKPOINT*/

#endif /*INCLUDED_CHECKPOINT_CONTAINER_VIEW_TRAVERSE_NDIM_H*/
