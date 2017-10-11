
#if ! defined INCLUDED_SERDES_DISPATCH_DESERIALIZER
#define INCLUDED_SERDES_DISPATCH_DESERIALIZER

#include "serdes_common.h"
#include "serdes_all.h"
#include "serializable_traits.h"

#include <type_traits>
#include <tuple>
#include <cstdlib>

namespace serdes {

template <typename SerializerT, typename T>
struct DeserializerDispatch {
  template <typename U>
  using isDefaultConsType =
    typename std::enable_if<std::is_default_constructible<U>::value, T>::type;

  template <typename U>
  using isNotDefaultConsType =
    typename std::enable_if<not std::is_default_constructible<U>::value, T>::type;

  // If we have the detection component, we can more precisely check for
  // reconstuctibility
  #if HAS_DETECTION_COMPONENT
    template <typename U>
    using isReconstructibleType =
      typename std::enable_if<SerializableTraits<U>::is_reconstructible, T>::type;
  #else
    template <typename U>
    using isReconstructibleType = isNotDefaultConsType<U>;
  #endif

  template <typename U = T>
  T& operator()(void* buf, isDefaultConsType<U>* x = nullptr) {
    debug_serdes("DeserializerDispatch: default constructor: buf=%p\n", buf);
    T* t_ptr = new (buf) T{};
    auto& t = *t_ptr;
    return t;
  }

  template <typename U = T>
  T& operator()(void* buf, isReconstructibleType<U>* x = nullptr) {
    debug_serdes("DeserializerDispatch: T::reconstruct(): buf=%p\n", buf);
    return T::reconstruct(buf);
  }
};

} //end namespace serdes

#endif /*INCLUDED_SERDES_DISPATCH_DESERIALIZER*/
