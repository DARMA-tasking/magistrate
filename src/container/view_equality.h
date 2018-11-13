
#if !defined INCLUDED_CONTAINER_VIEW_EQUALITY_H
#define INCLUDED_CONTAINER_VIEW_EQUALITY_H

#include "serdes_common.h"
#include "serializers/serializers_headers.h"
#include "container/view_traits_extract.h"
#include "container/view_traverse_manual.h"
#include "container/view_traverse_ndim.h"

#if KOKKOS_ENABLED_SERDES

#include <Kokkos_Core.hpp>
#include <Kokkos_View.hpp>
#include <Kokkos_DynamicView.hpp>
#include <Kokkos_Serial.hpp>

#include <array>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>
#include <tuple>
#include <type_traits>
#include <functional>

namespace serdes {

namespace detail {

/*
 * Impl class in detail for comparing static to dynamic extents on a single view
 */
template <typename ViewT, typename Callable>
struct ViewEqualityStatic {

  // Compare the static dimension to the dynamic extent: it must always be
  // equal for correctness
  template <typename T, unsigned N, unsigned M, typename... Args>
  void operator()(Kokkos::View<T*[N][M],Args...> const& v, Callable eq) {
    eq(v.extent(1), N);
    eq(v.extent(2), M);
  }

  template <typename T, unsigned N, unsigned M, typename... Args>
  void operator()(Kokkos::View<T[N][M],Args...> const& v, Callable eq) {
    eq(v.extent(0), N);
    eq(v.extent(1), M);
  }

  template <typename T, unsigned N, typename... Args>
  void operator()(Kokkos::View<T**[N],Args...> const& v, Callable eq) {
    eq(v.extent(2), N);
  }

  template <typename T, unsigned N, typename... Args>
  void operator()(Kokkos::View<T*[N],Args...> const& v, Callable eq) {
    eq(v.extent(1), N);
  }

  template <typename T, unsigned N, typename... Args>
  void operator()(Kokkos::View<T[N],Args...> const& v, Callable eq) {
    eq(v.extent(0), N);
  }

  template <typename AnyT, typename... Args>
  void operator()(Kokkos::View<AnyT,Args...> const& v, Callable eq) {
    // no static dimension match
  }

  template <typename AnyT, typename... Args>
  void operator()(
    Kokkos::Experimental::DynamicView<AnyT,Args...> const& v, Callable eq
  ) {
    // no static dimension match
  }

};

struct DefaultEQ {
  template <typename T>
  void operator()(T& a, T& b) const {
    assert(a == b && "Must be equal");
  }
  template <typename T>
  void operator()(T&& a, T&& b) const {
    assert(a == b && "Must be equal");
  }
};

} /* end namespace detail */

/*
 * Check equality of two views: meta-data, internal data, etc.
 *
 *   ========== Example w/default equality operator: =================
 *
 *     serdes::ViewEquality<ViewT>::compare(view1,view2);
 *
 *   ========== Example w/user-specified equality operator: ==========
 *
 *     struct EqFunctor {
 *       template <typename T>
 *       void operator()(T& a, T& b) const { EXPECT_EQ(a,b); }
 *       template <typename T>
 *       void operator()(T&& a, T&& b) const { EXPECT_EQ(a,b); }
 *     };
 *
 *     serdes::ViewEquality<ViewT>::template compare<EqFunctor>(view1,view2);
 *
 *  Note: if you specify your own equality functor, it needs to handle l-values
 *  and r-values based on how ViewEquality uses it. You can also specify a
 *  lambda or even two lambdas, one typed with the underlying data type of the
 *  view.
 *
 *  Depending on what your equality functor does, the comparison code will not
 *  stop when (in)equality is determined: it just invokes the function and does
 *  not have a return value to indicate incorrectness.
 *
 */

template <typename ViewT, typename ViewU = ViewT>
struct ViewEquality {
  using DataType     = typename serdes::ViewGetType<ViewT>::DataType;
  using CountDimType = serdes::CountDims<ViewT>;
  using BaseType     = typename CountDimType::BaseT;

  // Static check on the layout of the two views in question
  using LayoutT = typename ViewT::array_layout;
  using LayoutU = typename ViewU::array_layout;
  static_assert(std::is_same<LayoutT, LayoutU>::value, "Must be same layout");

  /*
   * This is the main compare call: it compares between two views the statically
   * specified extents in the type wrt to the dyanmic extents, extent equality
   * across two views, the meta-data in the view (label,span,etc.), and the
   * actual internal data inside the views by invoking the quality functor
   */
  template <typename DataEq = detail::DefaultEQ, typename MetaEq = DataEq>
  static void compare(
    ViewT const& v1, ViewU const& v2, DataEq eqd = {}, MetaEq eqm = {}
  ) {
    // Check static dimension wrt to dynamic extents for both views
    compareStaticDim(v1, eqm);
    compareStaticDim(v2, eqm);
    // Compare the extents of v1 and v2
    compareExtents(v1, v2, eqm);
    // Compare the meta-data for v1 and v2
    compareMeta(v1, v2, eqm);
    // Compare the actual data in v1 and v2
    compareData(v1, v2, eqd);
  }

  /*
   * This compares the equality for a single view of the static type-specified
   * extents to the dynamic extents to ensure equality
   */
  template <typename EqT = detail::DefaultEQ>
  static void compareStaticDim(ViewT const& v1, EqT c = {}) {
    detail::ViewEqualityStatic<ViewT,EqT> static_eq;
    static_eq.operator()(v1,c);
  }

  /*
   * This compares the extents of two views, including the static and dynamic
   * extents
   */
  template <typename EqT = detail::DefaultEQ>
  static void compareExtents(ViewT const& v1, ViewU const& v2, EqT eq = {}) {
    auto const ndims_v1 = CountDims<ViewT>::numDims(v1);
    auto const ndims_v2 = CountDims<ViewU>::numDims(v2);
    eq(ndims_v1,ndims_v2);
    for (auto i = 0; i < ndims_v1; i++) {
      eq(v1.extent(i), v2.extent(i));
    }
  }

  /*
   * This compares the view meta-data: size, label, contig, span
   */
  template <typename EqT = detail::DefaultEQ>
  static void compareMeta(ViewT const& v1, ViewU const& v2, EqT eq = {}) {
    eq(v1.label(),              v2.label());
    eq(v1.size(),               v2.size());
    eq(v1.span_is_contiguous(), v2.span_is_contiguous());
    eq(v1.span(),               v2.span());
  }

  /*
   * This compares the N-D internal data between two inputs that may be a
   * Kokkos::View<T> or Kokkos::DynamicView<T> (or conform to the interface). It
   * recursively traverses both of them and applies a supplied operator by
   * essentially zipping the elements
   */
  template <typename EqT = detail::DefaultEQ>
  static void compareData(ViewT const& v1, ViewU const& v2, EqT eq) {
    using DataType     = typename serdes::ViewGetType<ViewT>::DataType;
    using CountDimType = serdes::CountDims<ViewT>;
    using BaseType     = typename CountDimType::BaseT;
    using TupleType    = std::tuple<ViewT,ViewT>;

    constexpr auto dims = CountDimType::dynamic;

    // This is not strictly needed, but it might make this easier in the future
    auto fn = [eq](BaseType& elm1, BaseType& elm2){
      eq(elm1,elm2);
    };

    serdes::TraverseRecursive<TupleType,DataType,dims,decltype(fn)>::apply(
      std::make_tuple(v1,v2),fn
    );
  }

};

} /* end namespace serdes */

#endif /*KOKKOS_ENABLED_SERDES*/

#endif /*INCLUDED_CONTAINER_VIEW_EQUALITY_H*/
