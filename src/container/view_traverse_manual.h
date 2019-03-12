
#if !defined INCLUDED_CONTAINER_VIEW_TRAVERSE_MANUAL_H
#define INCLUDED_CONTAINER_VIEW_TRAVERSE_MANUAL_H

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

template <typename ViewT, typename = void>
struct SerializeMemory;

template <typename ViewT>
using KokkosAtomicArchetype = typename std::is_same<
  typename ViewT::traits::memory_traits, ::Kokkos::MemoryTraits<Kokkos::Atomic>
>;

template <typename ViewT>
struct SerializeMemory<
  ViewT,
  std::enable_if_t<KokkosAtomicArchetype<ViewT>::value>
>
{
  template <typename SerializerT>
  static void apply(SerializerT& s, ViewT const& v) {
    using DataType = typename ViewGetType<ViewT>::DataType;
    if (s.isUnpacking()) {
      DataType val;
      s | val;
      v.operator()().operator=(val);
    } else {
      auto const& val = v.operator()().operator const DataType();
      //DataType const& val = v.operator()();
      s | val;
    }
  }
};

template <typename ViewT>
struct SerializeMemory<
  ViewT,
  std::enable_if_t<!KokkosAtomicArchetype<ViewT>::value>
>
{
  template <typename SerializerT>
  static void apply(SerializerT& s, ViewT const& v) {
    s | v.operator()();
  }
};

template <typename SerializerT, typename ViewType, std::size_t dims>
struct TraverseManual;

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,0> {
  static void apply(SerializerT& s, ViewType const& v) {
    SerializeMemory<ViewType>::apply(s,v);

    //
    // This code works for both atomic and non-atomic views but I've specially
    // above for more clarity.
    //
    // if (s.isUnpacking()) {
    //   BaseType val;
    //   s | val;
    //   v.operator()() = val;
    // } else {
    //   /*
    //    * Be very careful here, do not use "auto const& val". If the view is
    //    * atomic we need to get the conversion operator to `BaseType`. In full
    //    * glory, we are invoking the following implicitly:
    //    *
    //    *   auto const& val = v.operator()().operator const BaseType();
    //    */
    //   BaseType const& val = v.operator()();
    //   s | val;
    // }
    //
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,1> {
  static void apply(SerializerT& s, ViewType const& v) {
    using BaseType = typename CountDims<ViewType>::BaseT;
    using SizeType = typename ViewType::size_type;
    for (SizeType i = 0; i < v.extent(0); i++) {
      if (s.isUnpacking()) {
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        BaseType val;
        s | val;
        v.operator()(i) = val;
      } else {
        BaseType const& val = v.operator()(i);
        s | val;
      }
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,2> {
  static void apply(SerializerT& s, ViewType const& v) {
    using BaseType = typename CountDims<ViewType>::BaseT;
    using SizeType = typename ViewType::size_type;
    for (SizeType i = 0; i < v.extent(0); i++) {
      for (SizeType j = 0; j < v.extent(1); j++) {
        if (s.isUnpacking()) {
          #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
          BaseType val;
          s | val;
          v.operator()(i,j) = val;
        } else {
          BaseType const& val = v.operator()(i,j);
          s | val;
        }
      }
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,3> {
  static void apply(SerializerT& s, ViewType const& v) {
    using BaseType = typename CountDims<ViewType>::BaseT;
    using SizeType = typename ViewType::size_type;
    for (SizeType i = 0; i < v.extent(0); i++) {
      for (SizeType j = 0; j < v.extent(1); j++) {
        for (SizeType k = 0; k < v.extent(2); k++) {
          if (s.isUnpacking()) {
            #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            BaseType val;
            s | val;
            v.operator()(i,j,k) = val;
          } else {
            BaseType const& val = v.operator()(i,j,k);
            s | val;
          }
        }
      }
    }
  }
};

template <typename SerializerT, typename ViewType>
struct TraverseManual<SerializerT,ViewType,4> {
  static void apply(SerializerT& s, ViewType const& v) {
    using BaseType = typename CountDims<ViewType>::BaseT;
    using SizeType = typename ViewType::size_type;
    for (SizeType i = 0; i < v.extent(0); i++) {
      for (SizeType j = 0; j < v.extent(1); j++) {
        for (SizeType k = 0; k < v.extent(2); k++) {
          for (SizeType l = 0; l < v.extent(3); l++) {
            if (s.isUnpacking()) {
              BaseType val;
              s | val;
              v.operator()(i,j,k,l) = val;
            } else {
              BaseType const& val = v.operator()(i,j,k,l);
              s | val;
            }
          }
        }
      }
    }
  }
};

} /* end namespace serdes */

#endif /*KOKKOS_ENABLED_SERDES*/

#endif /*INCLUDED_CONTAINER_VIEW_TRAVERSE_MANUAL_H*/
