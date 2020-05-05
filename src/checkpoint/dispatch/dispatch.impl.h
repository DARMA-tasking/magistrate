/*
//@HEADER
// *****************************************************************************
//
//                               dispatch.impl.h
//                           DARMA Toolkit v. 1.0.0
//                 DARMA/checkpoint => Serialization Library
//
// Copyright 2019 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// *****************************************************************************
//@HEADER
*/

#if !defined INCLUDED_SERDES_DISPATCH_IMPL
#define INCLUDED_SERDES_DISPATCH_IMPL

#include "checkpoint/serdes_common.h"
#include "checkpoint/dispatch/dispatch.h"

namespace serdes {

template <typename T>
SerialSizeType Dispatch<T>::sizeType(T& to_size) {
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
  PackerT& packer, T& to_pack, SerialSizeType const& size
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
  T& to_pack, SerialSizeType const& size, SerialByteType* buf
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
  SerialByteType* buf, SerialByteType* data, SerialSizeType const& size,
  bool in_place
) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;

  Unpacker unpacker(data, size);
  if (in_place) {
    auto t_buf = reinterpret_cast<T*>(buf);
    SerializerDispatch<Unpacker, CleanT> ap;
    ap(unpacker, t_buf, 1);
    return *t_buf;
  } else {
    DeserializerDispatch<Serializer, CleanT> apply_des;
    auto& target = apply_des(unpacker,buf);
    SerializerDispatch<Unpacker, CleanT> ap;
    ap(unpacker, &target, 1);
    return target;
  }
}

/* begin partial */
template <typename T>
SerialSizeType Dispatch<T>::sizeTypePartial(T& to_size) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(&to_size);
  Sizer sizer;
  SerializerDispatch<Sizer, CleanT> ap;
  ap.partial(sizer, val, 1);
  return sizer.getSize();
}

template <typename T>
template <typename PackerT>
BufferPtrType Dispatch<T>::packTypeWithPackerPartial(
  PackerT& packer, T& to_pack, SerialSizeType const& size
) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(&to_pack);
  SerializerDispatch<PackerT, CleanT> ap;
  ap.partial(packer, val, 1);
  return packer.extractPackedBuffer();
}

template <typename T>
BufferPtrType Dispatch<T>::packTypePartial(
  T& to_pack, SerialSizeType const& size, SerialByteType* buf
) {
  if (buf == nullptr) {
    Packer packer(size);
    return packTypeWithPackerPartial(packer, to_pack, size);
  } else {
    PackerUserBuf packer(size, std::make_unique<UserBuffer>(buf, size));
    return packTypeWithPackerPartial(packer, to_pack, size);
  }
}

template <typename T>
T& Dispatch<T>::unpackTypePartial(
  SerialByteType* buf, SerialByteType* data, SerialSizeType const& size
) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  Unpacker unpacker(data, size);
  SerializerDispatch<Unpacker, CleanT> ap;
  ap.partial(unpacker, reinterpret_cast<T*>(buf), 1);
  return *reinterpret_cast<T*>(buf);
}
/* end partial */

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
inline Serializer& operator&(Serializer& s, T& target) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(&target);

  if (s.isUnpacking()) {
    auto target_ptr = reinterpret_cast<char*>(val);
    T* new_target = new (target_ptr) T();
  }

  SerializerDispatch<Serializer, CleanT> ap;
  ap.partial(s, val, 1);

  return s;
}

template <typename Serializer, typename T>
inline void serializeArray(Serializer& s, T* array, SerialSizeType const num_elms) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(array);

  SerializerDispatch<Serializer, CleanT> ap;
  ap(s, val, num_elms);
}

template <typename Serializer, typename T>
inline void parserdesArray(Serializer& s, T* array, SerialSizeType const num_elms) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(array);

  SerializerDispatch<Serializer, CleanT> ap;
  ap.partial(s, val, num_elms);
}

template <typename T>
SerializedReturnType serializeType(T& to_serialize, BufferObtainFnType fn) {
  SerialSizeType size = Dispatch<T>::sizeType(to_serialize);
  debug_serdes("serializeType: size=%ld\n", size);
  SerialByteType* user_buf = fn ? fn(size) : nullptr;
  auto managed = Dispatch<T>::packType(to_serialize, size, user_buf);
  #if DEBUG_SERDES
    auto const& buf = managed->getBuffer();
    debug_serdes(
      "serializeType: buf=%p, size=%ld: val=%d\n",
      buf, size, *reinterpret_cast<int*>(buf)
    );
  #endif
  return std::make_tuple(std::move(managed), size);
}

template <typename T>
SerializedReturnType serializeTypePartial(
  T& to_serialize, BufferObtainFnType fn
) {
  SerialSizeType size = Dispatch<T>::sizeTypePartial(to_serialize);
  debug_serdes("serializeTypePartial: size=%ld\n", size);
  SerialByteType* user_buf = fn ? fn(size) : nullptr;
  auto managed = Dispatch<T>::packTypePartial(to_serialize, size, user_buf);
  auto const& buf = managed->getBuffer();
  debug_serdes(
    "serializeType (partial): buf=%p, size=%ld: val=%d\n",
    buf, size, *reinterpret_cast<int*>(buf)
  );
  return std::make_tuple(std::move(managed), size);
}

template <typename T>
T* deserializeType(
  SerialByteType* data, SerialSizeType const& size, T* allocBuf
) {
  auto mem = allocBuf ?
    reinterpret_cast<SerialByteType*>(allocBuf) : new SerialByteType[sizeof(T)];
  auto& t = Dispatch<T>::unpackType(mem, data, size);
  return &t;
}

template <typename T>
void deserializeType(InPlaceTag, SerialByteType* data, SerialSizeType sz, T* t) {
  auto t_place = reinterpret_cast<SerialByteType*>(t);
  Dispatch<T>::unpackType(t_place, data, sz, true);
}

template <typename T>
T* deserializeTypePartial(
  SerialByteType* data, SerialSizeType const& size, T* allocBuf
) {
  auto mem = allocBuf ?
    reinterpret_cast<SerialByteType*>(allocBuf) : new SerialByteType[sizeof(T)];
  auto& t = Dispatch<T>::unpackTypePartial(mem, data, size);
  return &t;
}

template <typename T>
std::size_t sizeType(T& target) {
  return Dispatch<T>::sizeType(target);
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_DISPATCH_IMPL*/
