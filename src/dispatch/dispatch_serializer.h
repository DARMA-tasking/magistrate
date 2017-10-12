
#if ! defined INCLUDED_SERDES_DISPATCH_SERIALIZER
#define INCLUDED_SERDES_DISPATCH_SERIALIZER

#include "serdes_common.h"
#include "serdes_all.h"
#include "traits/serializable_traits.h"

#include <type_traits>
#include <tuple>
#include <cstdlib>

namespace serdes {

template <typename SerializerT, typename T>
struct SerializerDispatch {
  template <typename U>
  using isByteCopyType =
  typename std::enable_if<std::is_arithmetic<U>::value, T>::type;

  template <typename U>
  using isNotByteCopyType =
  typename std::enable_if<not std::is_arithmetic<U>::value, T>::type;

  template <typename U = T>
  void operator()(SerializerT& s, T* val, SizeType num) {
    return apply(s, val, num);
  }

  // If we have the detection component, we can more precisely check for
  // serializability
  #if HAS_DETECTION_COMPONENT
    template <typename U>
    using hasInSerialize =
      typename std::enable_if<SerializableTraits<U>::has_serialize_instrusive, T>::type;
    template <typename U>
    using hasNoninSerialize =
      typename std::enable_if<SerializableTraits<U>::has_serialize_noninstrusive, T>::type;

    template <typename U = T>
    void apply(
      SerializerT& s, T* val, SizeType num, hasInSerialize<U>* x = nullptr
    ) {
      debug_serdes("SerializerDispatch: intrusive serialize: val=%p\n", &val);
      for (SizeType i = 0; i < num; i++) {
        val[i].template serialize<SerializerT>(s);
      }
    }

    template <typename U = T>
    void apply(
      SerializerT& s, T* val, SizeType num, hasNoninSerialize<U>* x = nullptr
    ) {
      debug_serdes("SerializerDispatch: non-intrusive serialize: val=%p\n", &val);
      for (SizeType i = 0; i < num; i++) {
        serialize(s, val[i]);
      }
    }

  #else
    template <typename U>
    using isSerializableType = isNotByteCopyType<U>;

    template <typename U = T>
    void apply(
      SerializerT& s, T* val, SizeType num, isSerializableType<U>* x = nullptr
    ) {
      debug_serdes("SerializerDispatch: serialize: val=%p\n", &val);
      for (SizeType i = 0; i < num; i++) {
        val[i].serialize(s);
      }
    }
  #endif

  template <typename U = T>
  void apply(
    SerializerT& s, T* val, SizeType num, isByteCopyType<U>* x = nullptr
  ) {
    debug_serdes("SerializerDispatch: contiguousTyped: val=%p\n", &val);
    SerializerT::contiguousTyped(s, val, num);
  }

};

} //end namespace serdes

#endif /*INCLUDED_SERDES_DISPATCH_SERIALIZER*/
