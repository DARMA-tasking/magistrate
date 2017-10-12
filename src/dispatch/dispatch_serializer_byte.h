
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

template <typename SerializerT, typename T>
struct SerializerDispatchByte {
  template <typename U>
  using isByteCopyType =
  typename std::enable_if<std::is_arithmetic<U>::value, T>::type;

  template <typename U>
  using isNotByteCopyType =
  typename std::enable_if<not std::is_arithmetic<U>::value, T>::type;

  template <typename U = T>
  void operator()(
    SerializerT& s, T* val, SizeType num, isByteCopyType<U>* x = nullptr
  ) {
    SerializerT::contiguousTyped(s, val, num);
  }

  template <typename U = T>
  void operator()(
    SerializerT& s, T* val, SizeType num, isNotByteCopyType<U>* x = nullptr
  ) {
    SerializerDispatchNonByte<SerializerT, T> dispatch;
    dispatch(s, val, num);
  }
};

} //end namespace serdes

#endif /*INCLUDED_SERDES_DISPATCH_SERIALIZER_BYTE*/
