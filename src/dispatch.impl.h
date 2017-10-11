
#if !defined INCLUDED_SERDES_DISPATCH_IMPL
#define INCLUDED_SERDES_DISPATCH_IMPL

#include "serdes_common.h"
#include "dispatch.h"

namespace serdes {

template <typename T>
SizeType Dispatch<T>::sizeType(T& to_size) {
  Sizer sizer;
  SerializerDispatch<Sizer, T> ap;
  ap(sizer, &to_size, 1);
  return sizer.getSize();
}

template <typename T>
ManagedBufferPtrType Dispatch<T>::packType(T& to_pack, SizeType const& size) {
  Packer packer(size);
  SerializerDispatch<Packer, T> ap;
  ap(packer, &to_pack, 1);
  return packer.extractPackedBuffer();
}

template <typename T>
T& Dispatch<T>::unpackType(
  SerialByteType* buf, SerialByteType* data, SizeType const& size
) {
  DeserializerDispatch<Serializer, T> apply_des;
  auto& target = apply_des(buf);
  Unpacker unpacker(data, size);
  SerializerDispatch<Unpacker, T> ap;
  ap(unpacker, &target, 1);
  return target;
}

template <typename Serializer, typename T>
inline void operator|(Serializer& s, T& target) {
  SerializerDispatch<Serializer, T> ap;
  ap(s, &target, 1);
}

template <typename Serializer, typename T>
inline void serializeArray(Serializer& s, T* array, SizeType const num_elms) {
  SerializerDispatch<Serializer, T> ap;
  ap(s, array, num_elms);
}

template <typename T>
ManagedSerializedType serializeType(T& to_serialize) {
  SizeType size = Dispatch<T>::sizeType(to_serialize);
  debug_serdes("serializeType: size=%ld\n", size);

  auto managed = Dispatch<T>::packType(to_serialize, size);
  auto const& buf = managed->getBuffer();
  debug_serdes("serializeType: buf=%p, size=%ld: val=%d\n", buf, size, *reinterpret_cast<int*>(buf));

  return std::make_tuple(std::move(managed), size);
}

template <typename T>
T& deserializeType(SerialByteType* data, SizeType const& size) {
  auto const& size_of_t = sizeof(T);
  auto mem = new SerialByteType[size_of_t];
  auto& t = Dispatch<T>::unpackType(mem, data, size);
  return t;
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_DISPATCH_IMPL*/
