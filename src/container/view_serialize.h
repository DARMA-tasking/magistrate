
#if !defined INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H
#define INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H

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
#include <cstdio>
#include <tuple>
#include <type_traits>

#define SERDES_DEBUG_ENABLED 0
#define CHECKPOINT_KOKKOS_PACK_LAYOUT 1

// I am shutting the n-dim traversal off by default for now, due to the extra
// template complexity that needs to be tested more extensively on different
// compiler versions
#define CHECKPOINT_KOKKOS_NDIM_TRAVERSE 0

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

  DEBUG_PRINT_SERDES(s, "serializeViewLabel: label=%s\n", view_label.c_str());

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

  DEBUG_PRINT_SERDES(
    s, "label=%s: chunk_size=%zu, max_extent=%zu, view_size=%zu\n",
    label.c_str(), chunk_size, max_extent, view_size
  );

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

  DEBUG_PRINT_SERDES(s, "label=%s: size=%zu\n", label.c_str(), view.size());

  // Serialize the Kokkos::DynamicView data manually by traversing with
  // DynamicView::operator()(...).
  //
  // @todo Optimize this by serializing by chunk
  //

#if CHECKPOINT_KOKKOS_NDIM_TRAVERSE
    using CountDimType = CountDims<ViewType>;
    using BaseType = typename CountDimType::BaseT;

    auto fn = [&s](BaseType& elm){
      s | elm;
    };

    TraverseRecursive<ViewType,T,1,decltype(fn)>::apply(view,fn);
#else
    TraverseManual<SerializerT,ViewType,1>::apply(s,view);
#endif
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
  auto const label = serializeViewLabel(s,view);

  // Serialize the total number of dimensions, including runtime+static dims
  int rt_dim = 0;
  if (!s.isUnpacking()) {
    rt_dim = CountDims<ViewType, T>::numDims(view);
  }
  s | rt_dim;

#if CHECKPOINT_KOKKOS_PACK_LAYOUT
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

  // Construct a view with the layout and use operator= to propagate out
  if (s.isUnpacking()) {
    view = constructView<ViewType>(label, nullptr, std::make_tuple(layout));
  }
#else
  //
  // This code for now is disabled by default
  //
  // Works only for Kokkos::LayoutLeft and Kokkos::LayoutRight
  //
  // Instead of serializing the layout struct data, serialize the extents that
  // get propagate to the View from the layout
  //
  // Note: enabling this option will *not* work with Kokkos::LayoutStride. It
  // will fail to compile with aa static_assert: because LayoutStide is not
  // extent constructible: traits::array_layout::is_extent_constructible!
  //
  constexpr auto dyn_dims = CountDims<ViewType, T>::dynamic;

  std::array<size_t, dyn_dims> extents_array;

  if (!s.isUnpacking()) {
    // Set up the extents array
    for (auto i = 0; i < dyn_dims; i++) {
      extents_array[i] = view.extent(i);
    }
  }

  s | extents_array;

  // Construct a view with the layout and use operator= to propagate out
  if (s.isUnpacking()) {
    view = constructView<ViewType>(label, nullptr, extents_array);
  }
#endif

  // Serialize the total number of elements in the Kokkos::View
  size_t num_elms = view.size();
  s | num_elms;

  // Serialize whether the view is contiguous or not. Is this required?
  bool is_contig = view.span_is_contiguous();
  s | is_contig;

  DEBUG_PRINT_SERDES(
    s, "label=%s: contig=%s, size=%zu, rt_dim=%d\n",
    label.c_str(), is_contig ? "true" : "false", num_elms, rt_dim
  );

  // Serialize the actual data owned by the Kokkos::View
  if (is_contig) {
    // Serialize the data directly out of the data buffer
    serializeArray(s, view.data(), num_elms);
  } else {
    // Serialize manually traversing the data with Kokkos::View::operator()(...)

#if CHECKPOINT_KOKKOS_NDIM_TRAVERSE
    using CountDimType = CountDims<ViewType>;
    using BaseType = typename CountDimType::BaseT;

    constexpr auto dims = CountDimType::dynamic;
    auto fn = [&s](BaseType& elm){
      s | elm;
    };

    TraverseRecursive<ViewType,T,dims,decltype(fn)>::apply(view,fn);
#else
    TraverseManual<SerializerT,ViewType,rank_val>::apply(s,view);
#endif
  }
}

} /* end namespace serdes */

#endif /*KOKKOS_ENABLED_SERDES*/

#endif /*INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H*/
