/*
//@HEADER
// *****************************************************************************
//
//                               view_serialize.h
//                           DARMA Toolkit v. 1.0.0
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

#if !defined INCLUDED_CHECKPOINT_CONTAINER_VIEW_SERIALIZE_H
#define INCLUDED_CHECKPOINT_CONTAINER_VIEW_SERIALIZE_H

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

#if KOKKOS_KERNELS_ENABLED
#include <Kokkos_StaticCrsGraph.hpp>
#include <KokkosSparse_CrsMatrix.hpp>
#endif

#include <array>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>
#include <cstdio>
#include <tuple>
#include <type_traits>

#define CHECKPOINT_DEBUG_ENABLED 0
#define CHECKPOINT_KOKKOS_PACK_LAYOUT 1

// I am shutting the n-dim traversal off by default for now, due to the extra
// template complexity that needs to be tested more extensively on different
// compiler versions
#define CHECKPOINT_KOKKOS_NDIM_TRAVERSE 0

#if CHECKPOINT_DEBUG_ENABLED
  #define DEBUG_PRINT_CHECKPOINT(ser, str, ...) do {                     \
      auto state = ser.isUnpacking() ? "Unpacking" : (               \
        ser.isSizing()               ? "Sizing"    : (               \
        ser.isPacking()              ? "Packing"   : "Invalid"));    \
        printf("mode=%s: " str, state,  __VA_ARGS__);                \
    } while (0);
#else
  #define DEBUG_PRINT_CHECKPOINT(str, ...)
#endif

namespace checkpoint {

/*
 * Serialization factory re-constructors for views taking a parameter pack for
 * the constructor.
 */

template <typename ViewType, std::size_t N,typename... I>
static ViewType buildView(
  std::string const& label, I&&... index
) {
  ViewType v{label, std::forward<I>(index)...};
  return v;
}

template <typename ViewType, unsigned Rank, typename Tuple, std::size_t... I>
static constexpr ViewType constructView(
  std::string const& view_label, Tuple&& t, std::index_sequence<I...>
) {
  return buildView<ViewType,Rank>(
    view_label,std::get<I>(std::forward<Tuple>(t))...
  );
}

template <typename ViewType, typename Tuple>
static constexpr ViewType constructView(
  std::string const& view_label, Tuple&& t
) {
  using TupUnrefT = std::remove_reference_t<Tuple>;
  constexpr auto tup_size = std::tuple_size<TupUnrefT>::value;
  return constructView<ViewType, ViewType::Rank>(
    view_label, std::forward<Tuple>(t), std::make_index_sequence<tup_size>{}
  );
}

/*
 * Factory for constructing a DynRankView
 */
template <typename ViewType, unsigned Rank, typename Tuple>
static constexpr ViewType constructRankedView(
  std::string const& view_label, Tuple&& t
) {
  using TupUnrefT = std::remove_reference_t<Tuple>;
  constexpr auto tup_size = std::tuple_size<TupUnrefT>::value;
  return constructView<ViewType, Rank>(
    view_label, std::forward<Tuple>(t), std::make_index_sequence<tup_size>{}
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

  DEBUG_PRINT_CHECKPOINT(s, "serializeViewLabel: label=%s\n", view_label.c_str());

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

  DEBUG_PRINT_CHECKPOINT(
    s, "label=%s: chunk_size=%zu, max_extent=%zu, view_size=%zu\n",
    label.c_str(), chunk_size, max_extent, view_size
  );

  // Reconstruct the view with the chunk size/extent information
  if (s.isUnpacking()) {
    unsigned min_chunk_size = static_cast<unsigned>(chunk_size);
    unsigned max_alloc_extent = static_cast<unsigned>(max_extent);
    view = constructView<ViewType>(
      label, std::make_tuple(min_chunk_size,max_alloc_extent)
    );

    // Resize the view to the size that was packed. It seems this is necessary.
    view.resize_serial(view_size);
  }

  DEBUG_PRINT_CHECKPOINT(s, "label=%s: size=%zu\n", label.c_str(), view.size());

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
inline void serialize_impl(SerializerT& s, Kokkos::DynRankView<T,Args...>& view) {
  using ViewType = Kokkos::DynRankView<T,Args...>;
  using ArrayLayoutType = typename ViewType::traits::array_layout;

  // Serialize the label for the view which is used to construct a new view with
  // the same label. Labels may not be unique and are for debugging Kokkos::View
  auto const label = serializeViewLabel(s,view);

  // Serialize the total number of runtime dimensions
  unsigned dims = 0;
  if (!s.isUnpacking()) {
    dims = view.rank();
  }
  s | dims;

  // Serialize the Kokkos layout data, including the extents, strides
  ArrayLayoutType layout;

  // Make sure we serialize all 7 dimensions, instead of just `dims`.
  if (s.isUnpacking()) {
    serializeLayout<SerializerT>(s, 7, layout);
  } else {
    ArrayLayoutType layout_cur = view.layout();

    // We must set these to the invalid index (they are not set in the layout by
    // default!). This ensures that when the DynRankView comes out after
    // de-serialization, the number of ranks is correct.
    for (int i = dims; i < 8; i++) {
      layout_cur.dimension[i] = KOKKOS_INVALID_INDEX;
    }

    serializeLayout<SerializerT>(s, 7, layout_cur);
  }

  // Construct a view with the layout and use operator= to propagate out
  if (s.isUnpacking()) {
    if (dims == 1) {
      view = constructRankedView<ViewType,1>(label, std::make_tuple(layout));
    } else if (dims == 2) {
      view = constructRankedView<ViewType,2>(label, std::make_tuple(layout));
    } else if (dims == 3) {
      view = constructRankedView<ViewType,3>(label, std::make_tuple(layout));
    } else if (dims == 4) {
      view = constructRankedView<ViewType,4>(label, std::make_tuple(layout));
    } else if (dims == 5) {
      view = constructRankedView<ViewType,5>(label, std::make_tuple(layout));
    } else if (dims == 6) {
      view = constructRankedView<ViewType,6>(label, std::make_tuple(layout));
    } else if (dims == 7) {
      view = constructRankedView<ViewType,7>(label, std::make_tuple(layout));
    } else {
      checkpointAssert(
        false,
        "Serializing Kokkos::DynRankView is only supported up to 7 dimensions"
      );
    }
  }

  // Serialize the total number of elements in the Kokkos::View
  size_t num_elms = view.size();
  s | num_elms;

  // Serialize whether the view is contiguous or not. Is this required?
  bool is_contig = view.span_is_contiguous();
  s | is_contig;

  DEBUG_PRINT_CHECKPOINT(
    s, "label=%s: contig=%s, size=%zu, dims=%d\n",
    label.c_str(), is_contig ? "true" : "false", num_elms, dims
  );

  bool init = false;
  if (!s.isUnpacking()) {
    init = (view.use_count() > 0 || view.data() != nullptr);
  }
  s | init;

  if (init) {
    // Serialize the actual data owned by the Kokkos::View
    if (is_contig) {
      // Serialize the data directly out of the data buffer
      dispatch::serializeArray(s, view.data(), num_elms);
    } else {
      if (dims == 1) {
        TraverseManual<SerializerT,ViewType,1>::apply(s,view);
      } else if (dims == 2) {
        TraverseManual<SerializerT,ViewType,2>::apply(s,view);
      } else if (dims == 3) {
        TraverseManual<SerializerT,ViewType,3>::apply(s,view);
      } else if (dims == 4) {
        TraverseManual<SerializerT,ViewType,4>::apply(s,view);
      } else if (dims == 5) {
        TraverseManual<SerializerT,ViewType,5>::apply(s,view);
      } else if (dims == 6) {
        TraverseManual<SerializerT,ViewType,6>::apply(s,view);
      } else if (dims == 7) {
        TraverseManual<SerializerT,ViewType,7>::apply(s,view);
      } else {
        checkpointAssert(
          false,
          "Serializing Kokkos::DynRankView is only supported up to 7 dimensions"
          " for non-contiguous views"
        );
      }
    }
  }
}

template <typename SerializerT, typename T, typename... Args>
inline void serialize_impl(SerializerT& s, Kokkos::View<T,Args...>& view) {
  using ViewType = Kokkos::View<T,Args...>;
  using ArrayLayoutType = typename ViewType::traits::array_layout;

  static constexpr auto const rank_val = ViewType::Rank;

  checkpointAssert(
    ViewType::traits::is_managed,
    "Serialization not implemented for unmanaged views"
  );

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
    view = constructView<ViewType>(label, std::make_tuple(layout));
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
  // will fail to compile with a static_assert: because LayoutStide is not
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
    view = constructView<ViewType>(label, extents_array);
  }
#endif

  // Serialize the total number of elements in the Kokkos::View
  size_t num_elms = view.size();
  s | num_elms;

  // Serialize whether the view is contiguous or not. Is this required?
  bool is_contig = view.span_is_contiguous();
  s | is_contig;

  DEBUG_PRINT_CHECKPOINT(
    s, "label=%s: contig=%s, size=%zu, rt_dim=%d\n",
    label.c_str(), is_contig ? "true" : "false", num_elms, rt_dim
  );

  bool init = false;
  if (!s.isUnpacking()) {
    init = (view.use_count() > 0 || view.data() != nullptr);
  }
  s | init;

  if (init) {
    auto host_view = Kokkos::create_mirror_view(view);
    if (s.isPacking()) {
      // Create and use an execution space to avoid a global Kokkos::fence()
      auto exec_space = Kokkos::HostSpace{};
      Kokkos::deep_copy(exec_space, host_view, view);
      exec_space.fence();
    }

    // Serialize the actual data owned by the Kokkos::View
    if (is_contig) {
      // Serialize the data directly out of the data buffer
      dispatch::serializeArray(s, host_view.data(), num_elms);
    } else {
      // Serialize manually traversing the data with Kokkos::View::operator()(...)

#if CHECKPOINT_KOKKOS_NDIM_TRAVERSE
      using CountDimType = CountDims<ViewType>;
      using BaseType = typename CountDimType::BaseT;

      constexpr auto dims = CountDimType::dynamic;
      auto fn = [&s](BaseType& elm){
        s | elm;
      };

      TraverseRecursive<ViewType,T,dims,decltype(fn)>::apply(host_view,fn);
#else
      TraverseManual<SerializerT,ViewType,rank_val>::apply(s,host_view);
#endif
    }

    if (s.isUnpacking()) {
      // Create and use an execution space to avoid a global Kokkos::fence()
      auto exec_space = Kokkos::HostSpace{};
      Kokkos::deep_copy(exec_space, view, host_view);
      exec_space.fence();
    }
  }
}

template <typename SerializerT, typename T, typename... Args>
inline void serialize_const(SerializerT& s, Kokkos::View<T,Args...>& view) {
  using ViewType = Kokkos::View<T,Args...>;
  using T_non_const = typename ViewType::traits::non_const_data_type;
  Kokkos::View<T_non_const,Args...> tmp_non_const(view.label(), view.layout());
  if (s.isPacking() || s.isSizing()) {
    Kokkos::deep_copy(tmp_non_const, view);
  }
  serialize_impl(s, tmp_non_const);
  if (s.isUnpacking()) {
    view = tmp_non_const;
  }
}

template <typename ViewT>
using KokkosConstArchetype = typename std::is_same<
  typename ViewT::traits::const_data_type, typename ViewT::traits::data_type
>;

template <typename ViewT, typename = void>
struct SerializeConst;

template <typename ViewT>
struct SerializeConst<
  ViewT,
  std::enable_if_t<!KokkosConstArchetype<ViewT>::value>
>
{
  template <typename SerializerT>
  static void apply(SerializerT& s, ViewT& v) {
    return serialize_impl(s,v);
  }
};

template <typename ViewT>
struct SerializeConst<
  ViewT,
  std::enable_if_t<KokkosConstArchetype<ViewT>::value>
>
{
  template <typename SerializerT>
  static void apply(SerializerT& s, ViewT& v) {
    return serialize_const(s, v);
  }
};

template <typename SerializerT, typename T, typename... Args>
inline void serialize(SerializerT& s, Kokkos::View<T,Args...>& view) {
  using ViewType = Kokkos::View<T,Args...>;
  SerializeConst<ViewType>::apply(s,view);
}

template <typename SerializerT, typename T, typename... Args>
inline void serialize(SerializerT& s, Kokkos::DynRankView<T,Args...>& view) {
  using ViewType = Kokkos::DynRankView<T,Args...>;
  SerializeConst<ViewType>::apply(s,view);
}

template <
  typename ViewType,
  typename T = typename ViewGetType<ViewType>::DataType
>
struct CountStaticDims {
  static constexpr int ndims = 0;
};

template <
  typename ViewType,
  typename T
>
struct CountStaticDims<ViewType, T*> {
  static constexpr int ndims = CountStaticDims<ViewType, T>::ndims + 1;
};

template <
  typename ViewType,
  typename T,
  std::size_t N
>
struct CountStaticDims<ViewType, T[N]> {
  static constexpr int ndims = CountStaticDims<ViewType, T>::ndims + 1;
};

template <typename SerializerT, typename ViewType, int N_DIMS>
struct SerializeExtentOnlyImpl;

template <typename SerializerT, typename ViewType>
struct SerializeExtentOnlyImpl<
  SerializerT,
  ViewType,
  1
> {
  static void apply(SerializerT& s, ViewType& v, std::string label) {
    // Pass label explicitly to reduce network transfer bytes
    auto view_extent_0 = v.extent(0);
    s | view_extent_0;
    if (s.isUnpacking()) {
      v = ViewType(label, view_extent_0);
    }
  }
};

template <typename SerializerT, typename ViewType>
struct SerializeExtentOnlyImpl<
  SerializerT,
  ViewType,
  2
> {
  static void apply(SerializerT& s, ViewType& v, std::string label) {
    // Pass label explicitly to reduce network transfer bytes
    auto view_extent_0 = v.extent(0);
    auto view_extent_1 = v.extent(1);
    s | view_extent_0 | view_extent_1;
    if (s.isUnpacking()) {
      v = ViewType(label, view_extent_0, view_extent_1);
    }
  }
};

template <typename SerializerT, typename ViewType>
struct SerializeExtentOnlyImpl<
  SerializerT,
  ViewType,
  3
> {
  static void apply(SerializerT& s, ViewType& v, std::string label) {
    // Pass label explicitly to reduce network transfer bytes
    auto view_extent_0 = v.extent(0);
    auto view_extent_1 = v.extent(1);
    auto view_extent_2 = v.extent(2);
    s | view_extent_0 | view_extent_1 | view_extent_2;
    if (s.isUnpacking()) {
      v = ViewType(label, view_extent_0, view_extent_1, view_extent_2);
    }
  }
};

template <typename SerializerT, typename ViewType>
struct SerializeExtentOnlyImpl<
  SerializerT,
  ViewType,
  4
> {
  static void apply(SerializerT& s, ViewType& v, std::string label) {
    // Pass label explicitly to reduce network transfer bytes
    auto view_extent_0 = v.extent(0);
    auto view_extent_1 = v.extent(1);
    auto view_extent_2 = v.extent(2);
    auto view_extent_3 = v.extent(3);
    s | view_extent_0 | view_extent_1 | view_extent_2 | view_extent_3;
    if (s.isUnpacking()) {
      v = ViewType(
        label, view_extent_0, view_extent_1, view_extent_2, view_extent_3
      );
    }
  }
};

template <typename SerializerT, typename ViewType>
void serializeExtentOnly(SerializerT& s, ViewType& v, std::string label) {
  constexpr int N_DIMS = CountStaticDims<ViewType>::ndims;
  SerializeExtentOnlyImpl<SerializerT, ViewType, N_DIMS>::apply(s, v, label);
}

template< typename SerializerT, typename T, typename... Ts >
void serializeContentsOnly(SerializerT& s, Kokkos::View<T, Ts...>& v) {
  Kokkos::View<T, Ts...> values = v;
  s | values;
  if (s.isUnpacking())
    Kokkos::deep_copy(v, values);
}

#if KOKKOS_KERNELS_ENABLED
template< typename Serializer, typename T, typename... Ts >
inline void serialize( Serializer &s, Kokkos::StaticCrsGraph<T, Ts...> &graph ) {
  s | graph.entries | graph.row_map | graph.row_block_offsets;
}

template< typename Serializer, typename T, typename... Ts >
inline void serialize( Serializer &s, KokkosSparse::CrsMatrix<T, Ts...> &matrix ) {
  using MatrixType = std::remove_reference_t<decltype(matrix)>;

  auto numCols = matrix.numCols();

  s | numCols;
  s | matrix.graph | matrix.values;

  if (s.isUnpacking()) {
    matrix = MatrixType(matrix.graph.entries.label(), numCols, matrix.values, matrix.graph);
  }
}
#endif

} /* end namespace checkpoint */

#endif /*KOKKOS_ENABLED_CHECKPOINT*/

#endif /*INCLUDED_CHECKPOINT_CONTAINER_VIEW_SERIALIZE_H*/
