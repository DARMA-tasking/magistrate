
#if !defined INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H
#define INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H

#include "serdes_common.h"
#include "serializers/serializers_headers.h"

#include <array>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>

namespace serdes {
namespace details {

template <typename SerializerT, typename ViewType, typename T>
struct Helper {
  static constexpr size_t dynamic_count = 0;
  static void countRunTimeDimensions( SerializerT& s, int & dim, ViewType view)
  {
    dim = 0;
  }
};

template <typename SerializerT, typename ViewType, typename T>
struct Helper<SerializerT, ViewType, T*> {
  static constexpr size_t dynamic_count =  Helper<SerializerT, ViewType, T>::dynamic_count +1;

  static void countRunTimeDimensions( SerializerT& s, int & dim, ViewType view)
  {
    Helper<SerializerT, ViewType, T>::countRunTimeDimensions(s, dim, view);
    size_t cur_extent = view.extent(dim);
    s | cur_extent;
    ++dim;
  }
};

template <typename SerializerT, typename ViewType, typename T, size_t N>
struct Helper<SerializerT, ViewType, T[N]> {
  static constexpr size_t dynamic_count =  Helper<SerializerT, ViewType, T>::dynamic_count;
  static void countRunTimeDimensions( SerializerT& s, int & dim, ViewType view)
  {
    Helper<SerializerT, ViewType, T>::countRunTimeDimensions(s, dim, view);
    ++dim;
  }
};
} // namespace detail


// DEPRECATED: Will be removed after unmaged view serialization is implemented
//template <typename ViewType, typename T, std::size_t N, std::size_t... I>
//static ViewType&& buildView(
//  std::size_t extents[N], std::string const& label, T* const val_ptr,
//  std::index_sequence<I...>
//) {
//  std::cout << "View construction : "<< N <<std::endl;
//  // If ``val_ptr'' is set then we will initialize an unmanaged view and pass it
//  // to the constructor
////  if (val_ptr != nullptr ) {
////    ViewType v{val_ptr,extents[I]...};
////    return std::move(v);
////  } else {
////    ViewType v{label,extents[I]...};
////    return std::move(v);
////  }
//  ViewType v{label,extents[I]...};
//  return std::move(v);
//}

template <typename ViewType, std::size_t N,typename... I>
static ViewType buildView(std::string const& label, typename ViewType::pointer_type const val_ptr,
                          I&&... index
                          ) {
  ViewType v{label, std::forward<I>(index)...};
  return v;
}

template <class ViewType, class Tuple, std::size_t... I>
constexpr decltype(auto) apply_impl(std::string const& view_label, typename ViewType::pointer_type const val_ptr, Tuple&& t, std::index_sequence<I...>)
{
  constexpr auto const rank_val = ViewType::Rank;

  return buildView<ViewType,rank_val>(
        view_label,val_ptr,std::get<I>(std::forward<Tuple>(t))...
        );
}

template <class ViewType, class Tuple>
constexpr decltype(auto) apply(std::string const&  view_label, typename ViewType::pointer_type const val_ptr,Tuple&& t)
{
  return apply_impl<ViewType>(
        view_label,val_ptr,
        std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
}

template <typename SerializerT, typename T, typename... Args>
inline void serialize(SerializerT& s, Kokkos::View<T,Args...>& view) {
  using ViewType = Kokkos::View<T,Args...>;
  using ViewTraitsType = Kokkos::ViewTraits<T,Args...>;

  static constexpr auto const rank_val = ViewType::Rank;
  static constexpr auto const is_managed = ViewType::traits::is_managed;

  // Serialize the label of the view
  std::string view_label;
  if (is_managed) {
    if (s.isUnpacking()) {
      s | view_label;
    } else {
      std::string label = view.label();
      s | label;
    }
  }

  int rtDim = 0;
  if (s.isUnpacking())
  {
    // Get the dynamic_count (number of runtime dimension) at deserialization
    details::Helper<SerializerT, ViewType, T>::dynamic_count;
  }
  else
  {
    // Compute the runtime extension at serialization
    details::Helper<SerializerT, ViewType, T>::countRunTimeDimensions(s, rtDim, view);
    std::cout << "countRunTimeDimensions: "<< rtDim <<std::endl;
  }

  std::array<size_t, details::Helper<SerializerT, ViewType, T>::dynamic_count> dynamicExtentsArray;
  if (s.isUnpacking())
  {
    s | dynamicExtentsArray;
  }

  if (is_managed) {
    if (s.isUnpacking()) {
      auto n_ = apply<ViewType>(
            view_label,nullptr, dynamicExtentsArray
            );
      view = n_;

      // Serialize the data out of the buffer directly into the internal
      // allocated memory
      serializeArray(s, view.data(), view.size());
    } else {
      for(int i = 0; i < 21; ++i)
      {
            std::cout << "data : " << i << " : " << view.data()[i]<<std::endl;
      }
      std::cout << "serializeArray of size "<< view.size() <<std::endl;
      serializeArray(s,  view.data(), 21);
      std::cout << "serializeArray OK " <<std::endl;
    }
  } else {
    // TODO: Implement the unmanaged view => See what can be combined and not combined with the managed one
    // Unmanaged view so we will allocate the memory and pass it to the
    // ViewType constructor
    //    if (s.isUnpacking()) {
    //      auto view_data_ = num_elms > 0 ? new T[num_elms]{} : nullptr;
    //       serializeArray(s, view_data_, num_elms);

    //      auto n_ = buildView<ViewType, T,rank_val>(
    //            extents_,view_label,view_data_,index_seq
    //            );
    //      view = n_;

    //      assert(num_elms == view.size() && "Num elements must equal size");
    //    } else {
    //      auto raw_ptr = num_elms > 0 ? view.data() : nullptr;
    //      serializeArray(s,  raw_ptr, num_elms);
    //    }
  }
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_CONTAINER_VIEW_SERIALIZE_H*/
