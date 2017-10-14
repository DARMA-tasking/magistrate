
#if !defined INCLUDED_SERDES_DISPATCH_IMPL
#define INCLUDED_SERDES_DISPATCH_IMPL

#include "serdes_common.h"
#include "dispatch/dispatch.h"

namespace serdes {

template <typename T>
SizeType Dispatch<T>::sizeType(T& to_size) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(&to_size);

  Sizer sizer;
  SerializerDispatch<Sizer, CleanT> ap;
  ap(sizer, val, 1);
  return sizer.getSize();
}

template <typename T>
template <typename PackerT>
BufferPtrType Dispatch<T>::packTypeWithPacker(
  PackerT& packer, T& to_pack, SizeType const& size
) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(&to_pack);

  SerializerDispatch<PackerT, CleanT> ap;
  ap(packer, val, 1);
  return packer.extractPackedBuffer();
}

template <typename T>
BufferPtrType Dispatch<T>::packType(
  T& to_pack, SizeType const& size, SerialByteType* buf
) {
  if (buf == nullptr) {
    Packer packer(size);
    return packTypeWithPacker(packer, to_pack, size);
  } else {
    PackerUserBuf packer(size, std::make_unique<UserBuffer>(buf, size));
    return packTypeWithPacker(packer, to_pack, size);
  }
}

template <typename T>
T& Dispatch<T>::unpackType(
  SerialByteType* buf, SerialByteType* data, SizeType const& size
) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;

  DeserializerDispatch<Serializer, CleanT> apply_des;
  auto& target = apply_des(buf);
  Unpacker unpacker(data, size);
  SerializerDispatch<Unpacker, CleanT> ap;
  ap(unpacker, &target, 1);
  return target;
}

template <typename Serializer, typename T>
inline Serializer& operator|(Serializer& s, T& target) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(&target);

  SerializerDispatch<Serializer, CleanT> ap;
  ap(s, val, 1);

  return s;
}

template <typename Serializer, typename T>
inline void serializeArray(Serializer& s, T* array, SizeType const num_elms) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(array);

  SerializerDispatch<Serializer, CleanT> ap;
  ap(s, val, num_elms);
}

template <typename T>
SerializedReturnType serializeType(T& to_serialize, BufferObtainFnType fn) {
  SizeType size = Dispatch<T>::sizeType(to_serialize);
  debug_serdes("serializeType: size=%ld\n", size);

  SerialByteType* user_buf = fn ? fn(size) : nullptr;
  auto managed = Dispatch<T>::packType(to_serialize, size, user_buf);
  auto const& buf = managed->getBuffer();
  debug_serdes("serializeType: buf=%p, size=%ld: val=%d\n", buf, size, *reinterpret_cast<int*>(buf));

  return std::make_tuple(std::move(managed), size);
}

template <typename T>
T* deserializeType(SerialByteType* data, SizeType const& size, T* allocBuf) {
  SerialByteType* mem = allocBuf ?
    reinterpret_cast<SerialByteType*>(allocBuf) : new SerialByteType[sizeof(T)];
  auto& t = Dispatch<T>::unpackType(mem, data, size);
  return &t;
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_DISPATCH_IMPL*/
