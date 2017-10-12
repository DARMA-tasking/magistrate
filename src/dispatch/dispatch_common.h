
#if !defined INCLUDED_SERDES_DISPATCH_COMMON
#define INCLUDED_SERDES_DISPATCH_COMMON

#include "serdes_common.h"
#include "buffer/buffer.h"
#include "dispatch_serializer.h"
#include "dispatch_deserializer.h"

#include <tuple>

namespace serdes {

template <typename T>
struct DispatchCommon {

  template <typename U>
  using isConst = typename std::enable_if<std::is_const<U>::value, T>::type;
  template <typename U>
  using isNotConst = typename std::enable_if<!std::is_const<U>::value, T>::type;

  using NonConstT = typename std::remove_const<T>::type;
  using NonConstRefT = typename std::decay<T>::type;
  using CleanT = NonConstRefT;

  template <typename U = T>
  static NonConstRefT* clean(T* val) {
    return DispatchCommon<T>::apply1(val);
  }

  template <typename U = T>
  static NonConstRefT* apply1(T* val, isConst<U>* x = nullptr) {
    return reinterpret_cast<NonConstRefT*>(const_cast<NonConstT*>(val));
  }

  template <typename U = T>
  static NonConstRefT* apply1(T* val, isNotConst<U>* x = nullptr) {
    return reinterpret_cast<NonConstRefT*>(val);
  }
};

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_DISPATCH_COMMON*/
