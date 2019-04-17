
#if !defined INCLUDED_SERIALIZE_INTERFACE_IMPL
#define INCLUDED_SERIALIZE_INTERFACE_IMPL

#include "serdes_common.h"
#include "serdes_headers.h"
#include "serialize_interface.h"
#include "buffer/buffer.h"

#include <memory>

namespace serialization { namespace interface {

template <typename T>
SerializedReturnType serialize(T& target, BufferCallbackType fn) {
  auto ret = ::serdes::serializeType<T>(target, fn);
  auto& buf = std::get<0>(ret);
  std::unique_ptr<SerializedInfo> base_ptr(
    static_cast<SerializedInfo*>(buf.release())
  );
  return base_ptr;
}

template <typename T>
T* deserialize(SerialByteType* buf, SizeType size, T* user_buf) {
  return ::serdes::deserializeType<T>(buf, size, user_buf);
}

template <typename T>
T* deserialize(SerializedReturnType&& in) {
  return ::serdes::deserializeType<T>(in->getBuffer(), in->getSize());
}

template <typename T>
void deserializeInPlace(SerialByteType* buf, SizeType size, T* t) {
  return ::serdes::deserializeType<T>(::serdes::InPlaceTag{}, buf, size, t);
}

template <typename T>
SerializedReturnType serializePartial(T& target, BufferCallbackType fn) {
  auto ret = ::serdes::serializeTypePartial<T>(target, fn);
  auto& buf = std::get<0>(ret);
  std::unique_ptr<SerializedInfo> base_ptr(
    static_cast<SerializedInfo*>(buf.release())
  );
  return base_ptr;
}

template <typename T>
T* deserializePartial(SerialByteType* buf, SizeType size, T* user_buf) {
  return ::serdes::deserializeTypePartial<T>(buf, size, user_buf);
}

template <typename T>
std::size_t getSize(T& target) {
  return ::serdes::sizeType<T>(target);
}

}} /* end namespace serialization::interface */

#endif /*INCLUDED_SERIALIZE_INTERFACE_IMPL*/
