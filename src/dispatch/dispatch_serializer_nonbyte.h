
#if ! defined INCLUDED_SERDES_DISPATCH_SERIALIZER_NONBYTE
#define INCLUDED_SERDES_DISPATCH_SERIALIZER_NONBYTE

#include "serdes_common.h"
#include "serdes_all.h"
#include "traits/serializable_traits.h"

#include <type_traits>
#include <tuple>
#include <cstdlib>

namespace serdes {

// Implement a lightweight simple detection mechanism (when full detector is not
// present) to detect type of serialize() method: intrusive/non-intrusive
// variants
#if !HAS_DETECTION_COMPONENT
template <typename SerializerT, typename T>
struct hasSerialize {
  template <
    typename C,
    typename = decltype(std::declval<C>().serialize(std::declval<SerializerT&>()))
  >
  static std::true_type test(int);

  template <typename C>
  static std::false_type test(...);

  static constexpr bool value = decltype(test<T>(0))::value;
};
#endif

template <typename SerializerT, typename T>
struct SerializerDispatchNonByte {

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
  #else
    template <typename U>
    using hasInSerialize =
    typename std::enable_if<hasSerialize<SerializerT, U>::value, T>::type;

    template <typename U>
    using hasNoninSerialize =
    typename std::enable_if<!hasSerialize<SerializerT, U>::value, T>::type;
  #endif

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
};

} //end namespace serdes

#endif /*INCLUDED_SERDES_DISPATCH_SERIALIZER_NONBYTE*/
