
#if !defined INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H
#define INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H

#include "serdes_common.h"
#include "serializers/serializers_headers.h"

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
#include <cstdio>

#define SERDES_DEBUG_ENABLED 0

#if SERDES_DEBUG_ENABLED
  #define DEBUG_PRINT_SERDES(ser, str, args...) do {                 \
      auto state = ser.isUnpacking() ? "Unpacking" : (               \
        ser.isSizing()               ? "Sizing"    : (               \
        ser.isPacking()              ? "Packing"   : "Invalid"));    \
        printf("mode=%s: " str, state, args);                        \
    } while (0);
#else
  #define DEBUG_PRINT_SERDES(str, args...)
#endif

namespace serdes {

/*
 * Serialization helper classes to count the number of runtime and static
 * dimensions of a Kokkos::View
 */

template <typename ViewType, typename T>
struct CountDims {
  static constexpr size_t dynamic = 0;
  static int numDims(ViewType const& view) { return 0; }
};

template <typename ViewType, typename T>
struct CountDims<ViewType, T*> {
  static constexpr size_t dynamic = CountDims<ViewType, T>::dynamic + 1;

  static int numDims(ViewType const& view) {
    auto const val = CountDims<ViewType, T>::numDims(view);
    return val + 1;
  }
};

template <typename ViewType, typename T, size_t N>
struct CountDims<ViewType, T[N]> {
  static constexpr size_t dynamic = CountDims<ViewType, T>::dynamic;
  static int numDims(ViewType const& view) {
    auto const val = CountDims<ViewType, T>::numDims(view);
    return val + 1;
  }
};

/*
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

/*
 * Serialization factory re-constructors for views taking a parameter pack for
 * the constructor.
 */

template <typename ViewType, std::size_t N,typename... I>
static ViewType buildView(
  std::string const& label, typename ViewType::pointer_type const val_ptr,
  I&&... index
) {
  ViewType v{label, std::forward<I>(index)...};
  return v;
}

template <typename ViewType, typename Tuple, std::size_t... I>
static constexpr ViewType constructView(
  std::string const& view_label, typename ViewType::pointer_type const val_ptr,
  Tuple&& t, std::index_sequence<I...>
) {
  constexpr auto const rank_val = ViewType::Rank;
  return buildView<ViewType,rank_val>(
    view_label,val_ptr,std::get<I>(std::forward<Tuple>(t))...
  );
}

template <typename ViewType, typename Tuple>
static constexpr ViewType constructView(
  std::string const& view_label, typename ViewType::pointer_type const val_ptr,
  Tuple&& t
) {
  using TupUnrefT = std::remove_reference_t<Tuple>;
  constexpr auto tup_size = std::tuple_size<TupUnrefT>::value;
  return constructView<ViewType>(
    view_label, val_ptr, std::forward<Tuple>(t),
    std::make_index_sequence<tup_size>{}
  );
}

/*
 * Serialization overloads for Kokkos::LayoutLeft, Kokkos::LayoutRight,
 * Kokkos::LayoutStride. Serialize the extents/stride in the Kokkos layout,
 * sufficient for proper reconstruction.
 */

template <typename SerdesT>
inline void serializeLayout(SerdesT& s, int dim, Kokkos::LayoutStride& layout) {
  for (auto i = 0; i < dim; i++) {
    s | layout.dimension[i];
    s | layout.stride[i];
  }
}

template <typename SerdesT>
inline void serializeLayout(SerdesT& s, int dim, Kokkos::LayoutLeft& layout) {
  for (auto i = 0; i < dim; i++) {
    s | layout.dimension[i];
  }
}

template <typename SerdesT>
inline void serializeLayout(SerdesT& s, int dim, Kokkos::LayoutRight& layout) {
  for (auto i = 0; i < dim; i++) {
    s | layout.dimension[i];
  }
}

template <typename SerializerT, typename ViewT>
inline std::string serializeViewLabel(SerializerT& s, ViewT& view) {
  // Serialize the label of the view
  std::string view_label = view.label();
  s | view_label;
  return view_label;
}

template <typename SerializerT, typename T, typename... Args>
inline void serialize(
  SerializerT& s, Kokkos::Experimental::DynamicView<T,Args...>& view
) {
  using ViewType = Kokkos::Experimental::DynamicView<T,Args...>;

  // Serialize the label for the view which is used to construct a new view with
  // the same label. Labels may not be unique and are for debugging Kokkos::View
  auto const label = serializeViewLabel(s,view);

  // Serialize the min chunk size, max extent, and view size which is used to
  // reconstruct the Kokkos::DynamicView
  std::size_t chunk_size = 0;
  std::size_t max_extent = 0;
  std::size_t view_size = 0;

  if (!s.isUnpacking()) {
    chunk_size = view.chunk_size();
    max_extent = view.allocation_extent();
    view_size = view.size();
  }

  s | chunk_size;
  s | max_extent;
  s | view_size;

  // std::cout << "chunk_size:" << chunk_size << std::endl;
  // std::cout << "max_extent:" << max_extent << std::endl;
  // std::cout << "view_size:" << view_size << std::endl;

  // Reconstruct the view with the chunk size/extent information
  if (s.isUnpacking()) {
    unsigned min_chunk_size = static_cast<unsigned>(chunk_size);
    unsigned max_alloc_extent = static_cast<unsigned>(max_extent);
    view = constructView<ViewType>(
      label, nullptr, std::make_tuple(min_chunk_size,max_alloc_extent)
    );

    // Resize the view to the size that was packed. It seems this is necessary.
    view.resize_serial(view_size);
  }

  // Serialize the Kokkos::DynamicView data manually by traversing with
  // DynamicView::operator()(...).
  //
  // @todo Optimize this by serializing by chunk
  //
  TraverseManual<SerializerT,ViewType,1>::apply(s,view);
}

template <typename SerializerT, typename T, typename... Args>
inline void serialize(SerializerT& s, Kokkos::View<T,Args...>& view) {
  using ViewType = Kokkos::View<T,Args...>;
  using ViewTraitsType = Kokkos::ViewTraits<T,Args...>;
  using ArrayLayoutType = typename ViewType::traits::array_layout;

  static constexpr auto const rank_val = ViewType::Rank;
  static constexpr auto const is_managed = ViewType::traits::is_managed;

  assert(is_managed && "Serialization not implemented for unmanaged views");

  // Serialize the label for the view which is used to construct a new view with
  // the same label. Labels may not be unique and are for debugging Kokkos::View
  auto const view_label = serializeViewLabel(s,view);

  // Serialize the total number of dimensions, including runtime+static dims
  int rt_dim = 0;
  if (!s.isUnpacking()) {
    rt_dim = CountDims<ViewType, T>::numDims(view);
  }
  s | rt_dim;

  // Serialize the Kokkos layout data, including the extents, strides
  ArrayLayoutType layout;

  // This is ordered as so because the view.layout() might fail before proper
  // initialization
  if (s.isUnpacking()) {
    serializeLayout<SerializerT>(s, rt_dim, layout);
  } else {
    ArrayLayoutType layout_cur = view.layout();
    serializeLayout<SerializerT>(s, rt_dim, layout_cur);
  }

  // Serialize the total number of elements in the Kokkos::View
  size_t num_elms = view.size();
  s | num_elms;

  // Construct a view with the layout and use operator= to propagate out
  if (s.isUnpacking()) {
    view = constructView<ViewType>(view_label, nullptr, std::make_tuple(layout));
  }

  // Serialize whether the view is contiguous or not. Is this required?
  bool is_contig = view.span_is_contiguous();
  s | is_contig;

  // Serialize the actual data owned by the Kokkos::View
  if (is_contig) {
    // Serialize the data directly out of the data buffer
    serializeArray(s, view.data(), num_elms);
  } else {
    // Serialize manually traversing the data with Kokkos::View::operator()(...)
    TraverseManual<SerializerT,ViewType,rank_val>::apply(s,view);
  }
}

} /* end namespace serdes */

#endif /*KOKKOS_ENABLED_SERDES*/

#endif /*INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H*/
