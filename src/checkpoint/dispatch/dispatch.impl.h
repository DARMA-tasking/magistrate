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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_IMPL_H
#define INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_IMPL_H

#include "checkpoint/common.h"
#include "checkpoint/dispatch/dispatch.h"

namespace checkpoint {

template <typename Serializer, typename T>
inline Serializer& operator|(Serializer& s, T& target) {
  using DispatchT = dispatch::DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(&target);

  dispatch::SerializerDispatch<Serializer, CleanT> ap;
  ap(s, val, 1);

  return s;
}

} /* end namespace checkpoint */

namespace checkpoint { namespace dispatch {

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
buffer::BufferPtrType Dispatch<T>::packTypeWithPacker(
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
buffer::BufferPtrType Dispatch<T>::packType(
  T& to_pack, SerialSizeType const& size, SerialByteType* buf
) {
  if (buf == nullptr) {
    Packer packer(size);
    return packTypeWithPacker(packer, to_pack, size);
  } else {
    PackerUserBuf packer(size, std::make_unique<buffer::UserBuffer>(buf, size));
    return packTypeWithPacker(packer, to_pack, size);
  }
}

template <typename T>
T& Dispatch<T>::unpackType(
  SerialByteType* buf, SerialByteType* data, bool in_place
) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;

  Unpacker unpacker(data);
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

template <typename Serializer, typename T>
inline void serializeArray(Serializer& s, T* array, SerialSizeType const num_elms) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(array);

  SerializerDispatch<Serializer, CleanT> ap;
  ap(s, val, num_elms);
}

template <typename T>
buffer::ImplReturnType serializeType(T& to_serialize, BufferObtainFnType fn) {
  SerialSizeType size = Dispatch<T>::sizeType(to_serialize);
  debug_checkpoint("serializeType: size=%ld\n", size);
  SerialByteType* user_buf = fn ? fn(size) : nullptr;
  auto managed = Dispatch<T>::packType(to_serialize, size, user_buf);
  #if DEBUG_CHECKPOINT
    auto const& buf = managed->getBuffer();
    debug_checkpoint(
      "serializeType: buf=%p, size=%ld: val=%d\n",
      buf, size, *reinterpret_cast<int*>(buf)
    );
  #endif
  return std::make_tuple(std::move(managed), size);
}

template <typename T>
T* deserializeType(SerialByteType* data, T* allocBuf) {
  auto mem = allocBuf ?
    reinterpret_cast<SerialByteType*>(allocBuf) : new SerialByteType[sizeof(T)];
  auto& t = Dispatch<T>::unpackType(mem, data);
  return &t;
}

template <typename T>
void deserializeType(InPlaceTag, SerialByteType* data, T* t) {
  auto t_place = reinterpret_cast<SerialByteType*>(t);
  Dispatch<T>::unpackType(t_place, data, true);
}

template <typename T>
std::size_t sizeType(T& target) {
  return Dispatch<T>::sizeType(target);
}

}} /* end namespace checkpoint::dispatch */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_IMPL_H*/
