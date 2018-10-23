
#if !defined INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H
#define INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H

#include "serdes_common.h"
#include "serializers/serializers_headers.h"

#include <type_traits>
#include <cassert>

namespace serdes {

template <typename ViewType, typename T, std::size_t N, std::size_t... I>
static ViewType&& buildView(
  std::size_t extents[N], std::string const& label, T* const val_ptr,
  std::index_sequence<I...>
) {
  // If ``val_ptr'' is set then we will initialize an unmanaged view and pass it
  // to the constructor
  if (val_ptr != nullptr) {
    ViewType v{label,val_ptr,extents[I]...};
    return std::move(v);
  } else {
    ViewType v{label,extents[I]...};
    return std::move(v);
  }
}

template <typename SerializerT, typename T, typename... Args>
inline void serialize(SerializerT& s, Kokkos::View<T,Args...>& view) {
  using ViewType = Kokkos::View<T,Args...>;

  static constexpr auto const rank_val = ViewType::Rank;
  static constexpr auto const is_managed = ViewType::traits::is_managed;

  // Serialize whether the underlying data pointer is null
  bool is_null_ptr;
  if (s.isUnpacking()) {
    s | is_null_ptr;
  } else {
    bool view_is_null = view.data() == nullptr;
    s | view_is_null;
  }

  if (!is_null_ptr) {
    // Serialize whether it is contiguous or not
    bool is_contig;
    if (s.isUnpacking()) {
      s | is_contig;
    } else {
      bool view_is_contig = view.span_is_contiguous();
      s | view_is_contig;
    }

    // Serialize number of elements
    size_t num_elms;
    if (s.isUnpacking()) {
      s | num_elms;
    } else {
      size_t view_num_elms = view.size();
      s | view_num_elms;
    }

    // Serialize the label of the view
    std::string view_label;
    if (s.isUnpacking()) {
      s | view_label;
    } else {
      size_t label = view.label();
      s | label;
    }

    // For each dimension, serialize the extents
    size_t extents_[rank_val];
    if (s.isUnpacking()) {
      for (int i = 0; i < rank_val; i++) {
        s | extents_[i];
      }
    } else {
      for (int i = 0; i < rank_val; i++) {
        size_t cur_extent = view.extent(i);
        s | cur_extent;
      }
    }

    // Index sequence for unwinding the extent array into the constructor
    static constexpr auto index_seq = std::make_index_sequence<rank_val>{};

    if (is_contig) {
      if (is_managed) {
        if (s.isUnpacking()) {
          // Construct ViewType and use operator= to set the output ref ``view''
          auto n_ = buildView<ViewType,T,rank_val>(
            extents_,view_label,nullptr,index_seq
          );
          view = n_;

          T& view_internal_data = view.operator()();
          T* view_internal_ptr = &view_internal_data;

          assert(num_elms == view.size() && "Num elements must equal size");

          // Serialize the data out of the buffer directly into the internal
          // allocated memory
          serializeArray(s, view_internal_ptr, num_elms);
        } else {
          T* raw_ptr = view.data();
          serializeArray(s, raw_ptr, num_elms);
        }
      } else {
        // Unmanaged view so we will allocate the memory and pass it to the
        // ViewType constructor
        if (s.isUnpacking()) {
          auto view_data_ = new T[num_elms]{};
          serializeArray(s, view_data_, num_elms);

          auto n_ = buildView<ViewType,T,rank_val>(
            extents_,view_label,view_data_,index_seq
          );
          view = n_;

          assert(num_elms == view.size() && "Num elements must equal size");
        } else {
          T* raw_ptr = view.data();
          serializeArray(s, raw_ptr, num_elms);
        }
      }
    } else {
      // Serialize the strides in each dimension for the view: ``rank_val'' is a
      // statically known value.
      // int64_t strides[rank_val + 1];

      // if (s.isUnpacking()) {
      //   for (int i = 0; i < rank_val; i++) {
      //     s | strides[i];
      //   }
      //   // By passing a pointer, Kokkos assigns the strides to the view
      //   view.stride(strides);
      // } else {
      //   for (int i = 0; i < rank_val; i++) {
      //     // Get the stride for the view during packing for each dimension
      //     s | view.stride(i);
      //   }
      // }
    }
  }
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H*/
