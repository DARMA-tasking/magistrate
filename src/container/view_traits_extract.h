
#if !defined INCLUDED_CONTAINER_VIEW_TRAITS_EXTRACT_H
#define INCLUDED_CONTAINER_VIEW_TRAITS_EXTRACT_H

#include "serdes_common.h"
#include "serializers/serializers_headers.h"

#if KOKKOS_ENABLED_SERDES

#include <Kokkos_Core.hpp>
#include <Kokkos_View.hpp>
#include <Kokkos_DynamicView.hpp>
#include <Kokkos_Serial.hpp>

#include <utility>
#include <tuple>
#include <type_traits>

namespace serdes {

/*
 * Serialization helper classes to count the number of runtime and static
 * dimensions of a Kokkos::View
 */

// This shouldn't be necessary but I can't find the correct trait to extract in
// Kokkos::View, so I will manually extract the underlying data type
template <typename ViewType>
struct ViewGetType;

template <typename T, typename... Args>
struct ViewGetType<Kokkos::View<T,Args...>> {
  using DataType = T;
};

template <typename T, typename... Args>
struct ViewGetType<Kokkos::Experimental::DynamicView<T,Args...>> {
  using DataType = T;
};

template <
  typename ViewType,
  typename T = typename ViewGetType<ViewType>::DataType
>
struct CountDims {
  using BaseT = typename std::decay<T>::type;
  static constexpr size_t dynamic = 0;
  static int numDims(ViewType const& view) { return 0; }
};

template <typename ViewType, typename T>
struct CountDims<ViewType, T*> {
  using BaseT = typename CountDims<ViewType,T>::BaseT;
  static constexpr size_t dynamic = CountDims<ViewType, T>::dynamic + 1;

  static int numDims(ViewType const& view) {
    auto const val = CountDims<ViewType, T>::numDims(view);
    return val + 1;
  }
};

template <typename ViewType, typename T, size_t N>
struct CountDims<ViewType, T[N]> {
  using BaseT = typename CountDims<ViewType,T>::BaseT;
  static constexpr size_t dynamic = CountDims<ViewType, T>::dynamic;
  static int numDims(ViewType const& view) {
    auto const val = CountDims<ViewType, T>::numDims(view);
    return val + 1;
  }
};

} /* end namespace serdes */

#endif /*KOKKOS_ENABLED_SERDES*/

#endif /*INCLUDED_CONTAINER_VIEW_TRAITS_EXTRACT_H*/
