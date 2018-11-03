
#if !defined INCLUDED_CONTAINER_VIEW_TRAVERSE_MANUAL_H
#define INCLUDED_CONTAINER_VIEW_TRAVERSE_MANUAL_H

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
 *                     === Manual Traversal ===
 *
 * This is left here to test the automatic code.
 *
 * Serialization of data backing a view by manually traversing the data using
 * the Kokkos::View::operator()(...) or
 * Kokkos::DynamicView::operator()(...). The access operator is guaranteed to
 * correctly accesses the data regardless of the layout.
 */

template <typename SerializerT, typename ViewType, std::size_t dims>
struct TraverseManual;

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,1> {
  static void apply(SerializerT& s, ViewType const& v) {
    for (typename ViewType::size_type i = 0; i < v.extent(0); i++) {
      s | v.operator()(i);
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,2> {
  static void apply(SerializerT& s, ViewType const& v) {
    for (typename ViewType::size_type i = 0; i < v.extent(0); i++) {
      for (typename ViewType::size_type j = 0; j < v.extent(1); j++) {
        s | v.operator()(i,j);
      }
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,3> {
  static void apply(SerializerT& s, ViewType const& v) {
    for (typename ViewType::size_type i = 0; i < v.extent(0); i++) {
      for (typename ViewType::size_type j = 0; j < v.extent(1); j++) {
        for (typename ViewType::size_type k = 0; k < v.extent(2); k++) {
          s | v.operator()(i,j,k);
        }
      }
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,4> {
  static void apply(SerializerT& s, ViewType const& v) {
    for (typename ViewType::size_type i = 0; i < v.extent(0); i++) {
      for (typename ViewType::size_type j = 0; j < v.extent(1); j++) {
        for (typename ViewType::size_type k = 0; k < v.extent(2); k++) {
          for (typename ViewType::size_type l = 0; l < v.extent(3); l++) {
            s | v.operator()(i,j,k,l);
          }
        }
      }
    }
  }
};

} /* end namespace serdes */

#endif /*KOKKOS_ENABLED_SERDES*/

#endif /*INCLUDED_CONTAINER_VIEW_TRAVERSE_MANUAL_H*/
