
#if !defined INCLUDED_CONTAINER_VIEW_TRAVERSE_NDIM_H
#define INCLUDED_CONTAINER_VIEW_TRAVERSE_NDIM_H

#include "serdes_common.h"
#include "serializers/serializers_headers.h"
#include "container/view_traits_extract.h"

#if KOKKOS_ENABLED_SERDES

#include <Kokkos_Core.hpp>
#include <Kokkos_View.hpp>
#include <Kokkos_DynamicView.hpp>
#include <Kokkos_Serial.hpp>

#include <utility>
#include <tuple>
#include <cassert>
#include <type_traits>

namespace serdes {


/*
 * Traversal of data backing a view by recursively traversing each dimension and
 * every element of the data one-by-one using the Kokkos::View::operator()(...)
 * or Kokkos::DynamicView::operator()(...). The access operator is guaranteed to
 * correctly accesses the data regardless of the layout. This new code does a
 * recursive traversal so it works for n-dimensional views.
 *
 * This code generally allows you to specify any callable, and use two views if
 * you want to operate by essentially zipping the elements and passing to the
 * call function.
 *
 */

// Recursive callable apply for n-dimension automatic traversal of a view
template <
  typename ViewT, typename T, typename TupleT, unsigned d, typename Callable
>
struct RecurDimTraverse;

// Simple meta-code for detecting a tuple type
template <typename T>
struct IsTuple : std::false_type {};
template <typename... U>
struct IsTuple<std::tuple<U...>> : std::true_type {};

template <typename T>
struct GetViewSizeType {
  using SizeType = typename T::size_type;
};

template <typename T, typename W>
struct GetViewSizeType<std::tuple<T,W>> {
  using SizeType = typename T::size_type;
};

// General code for traversing a static or dynamic dimension and recursively
// preceding pushing the index into the tuple
template <
  typename ViewT, typename T, typename TupleT, unsigned d, typename Callable
>
struct RecurBaseTraverseDim {
  using SizeType = typename GetViewSizeType<ViewT>::SizeType;

  using TupleNextT = decltype(
    std::tuple_cat(
      std::declval<TupleT>(),
      std::declval<std::tuple<SizeType>>()
    )
  );
  using RecurT = RecurDimTraverse<ViewT,T,TupleNextT,d-1,Callable>;

  template <typename U>
  using ViewTupleT = typename std::enable_if<IsTuple<U>::value,ViewT>::type;
  template <typename U>
  using NotViewTupleT = typename std::enable_if<!IsTuple<U>::value,ViewT>::type;

  template <typename U = ViewT>
  static void applyImpl(
    ViewT const& view, unsigned nd, TupleT idx, Callable call,
    ViewTupleT<U>* _x = nullptr
  ) {
    auto const ex1 = std::get<0>(view).extent(nd-d);
    auto const ex2 = std::get<1>(view).extent(nd-d);
    assert(ex1 == ex2 && "Matching extents must be equal");
    for (SizeType i = 0; i < ex1; i++) {
      RecurT::apply(view,nd,std::tuple_cat(idx,std::tuple<SizeType>(i)),call);
    }
  }

  template <typename U = ViewT>
  static void applyImpl(
    ViewT const& view, unsigned nd, TupleT idx, Callable call,
    NotViewTupleT<U>* _x = nullptr
  ) {
    for (SizeType i = 0; i < view.extent(nd-d); i++) {
      RecurT::apply(view,nd,std::tuple_cat(idx,std::tuple<SizeType>(i)),call);
    }
  }

  static void apply(ViewT const& view, unsigned nd, TupleT idx, Callable call) {
    applyImpl(view,nd,idx,call);
  }
};

// Overload for traversing a static T[N] dimension
template <
  typename ViewT, typename T, unsigned N, typename TupleT, unsigned d,
  typename Callable
>
struct RecurDimTraverse<ViewT, T[N], TupleT, d, Callable>
  : RecurBaseTraverseDim<ViewT,T,TupleT,d,Callable>
{ };

// Overload for traversing a dynamic (T*) dimension
template <
  typename ViewT, typename T, typename TupleT, unsigned d, typename Callable
>
struct RecurDimTraverse<ViewT, T*, TupleT, d, Callable>
  : RecurBaseTraverseDim<ViewT,T,TupleT,d,Callable>
{ };

// Overload for traversing after we reach recursively the last dimension and now
// will dispatch the View::operator()()
template <
  typename ViewT, typename T, typename TupleT, unsigned d, typename Callable
>
struct RecurDimTraverse {
  using SizeType = typename GetViewSizeType<ViewT>::SizeType;

  using TupleNextT = decltype(std::declval<std::tuple<>>());

  template <typename U>
  using ViewTupleT = typename std::enable_if<IsTuple<U>::value,ViewT>::type;
  template <typename U>
  using NotViewTupleT = typename std::enable_if<!IsTuple<U>::value,ViewT>::type;

  template <typename U, std::size_t... I>
  static void dispatchImpl(
    ViewT const& view, Callable call, TupleT tup, std::index_sequence<I...>,
    ViewTupleT<U>* _x = nullptr
  ) {
    // Dispatch the call operator on both elements of the view tuple, used for
    // comparison between the elements
    call(
      std::get<0>(view).operator()(std::get<I>(tup)...),
      std::get<1>(view).operator()(std::get<I>(tup)...)
    );
  }

  template <typename U, std::size_t... I>
  static void dispatchImpl(
    ViewT const& view, Callable call, TupleT tup, std::index_sequence<I...>,
    NotViewTupleT<U>* _x = nullptr
  ) {
    // Dispatch the call operator on the view
    call(view.operator()(std::get<I>(tup)...));
  }


  template <std::size_t... I>
  static void dispatch(
    ViewT const& view, Callable call, TupleT tup, std::index_sequence<I...> idx
  ) {
    dispatchImpl<ViewT,I...>(view,call,tup,idx);
  }

  static void dispatch(ViewT const& view, Callable call, TupleT idx_tup) {
    constexpr auto tup_size = std::tuple_size<TupleT>::value;
    return dispatch(view,call,idx_tup,std::make_index_sequence<tup_size>{});
  }

  static void apply(ViewT const& view, unsigned, TupleT idx, Callable call) {
    return dispatch(view,call,idx);
  }
};

template <typename ViewT, typename T, unsigned nd, typename Callable>
struct TraverseRecur {
  using RecurImplT = RecurDimTraverse<ViewT,T,std::tuple<>,nd,Callable>;
  static void apply(ViewT const& view, Callable call) {
    return RecurImplT::apply(view,nd,std::tuple<>{},call);
  }
};

} /* end namespace serdes */

#endif /*KOKKOS_ENABLED_SERDES*/

#endif /*INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H*/
