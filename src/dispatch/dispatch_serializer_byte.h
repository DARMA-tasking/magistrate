
#if ! defined INCLUDED_SERDES_DISPATCH_SERIALIZER_BYTE
#define INCLUDED_SERDES_DISPATCH_SERIALIZER_BYTE

#include "serdes_common.h"
#include "serdes_all.h"
#include "traits/serializable_traits.h"
#include "dispatch_serializer_nonbyte.h"

#include <type_traits>
#include <tuple>
#include <cstdlib>

namespace serdes {

#if !HAS_DETECTION_COMPONENT
template <typename T>
struct hasByteCopy {
  template <typename C, typename = typename C::isByteCopyable>
  static typename C::isByteCopyable test(int);

  template <typename C>
  static std::false_type test(...);

  static constexpr bool value = decltype(test<T>(0))::value;
};
#endif

template <typename SerializerT, typename T>
struct SerializerDispatchByte {
  #if HAS_DETECTION_COMPONENT
    template <typename U>
    using isByteCopyType =
    typename std::enable_if<SerializableTraits<U>::is_bytecopyable, T>::type;

    template <typename U>
    using isNotByteCopyType =
    typename std::enable_if<not SerializableTraits<U>::is_bytecopyable, T>::type;
  #else
    template <typename U>
    using isByteCopyType =
    typename std::enable_if<
      std::is_arithmetic<U>::value or hasByteCopy<U>::value, T
    >::type;

    template <typename U>
    using isNotByteCopyType =
    typename std::enable_if<
      not std::is_arithmetic<U>::value and not hasByteCopy<U>::value, T
    >::type;
  #endif

  template <typename U = T>
  void operator()(
    SerializerT& s, T* val, SerialSizeType num, isByteCopyType<U>* x = nullptr
  ) {
    SerializerT::contiguousTyped(s, val, num);
  }

  template <typename U = T>
  void operator()(
    SerializerT& s, T* val, SerialSizeType num, isNotByteCopyType<U>* x = nullptr
  ) {
    SerializerDispatchNonByte<SerializerT, T> dispatch;
    dispatch(s, val, num);
  }

  template <typename U = T>
  void partial(
    SerializerT& s, T* val, SerialSizeType num, isByteCopyType<U>* x = nullptr
  ) {
    SerializerT::contiguousTyped(s, val, num);
  }

  template <typename U = T>
  void partial(
    SerializerT& s, T* val, SerialSizeType num, isNotByteCopyType<U>* x = nullptr
  ) {
    SerializerDispatchNonByteParserdes<SerializerT, T> dispatch;
    dispatch.partial(s, val, num);
  }
};

} //end namespace serdes

#endif /*INCLUDED_SERDES_DISPATCH_SERIALIZER_BYTE*/
