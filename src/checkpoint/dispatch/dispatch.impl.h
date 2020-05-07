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
void Dispatch<T>::packTypeWithPacker(
  PackerT& packer, T& to_pack, SerialSizeType const& size
) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(&to_pack);

  SerializerDispatch<PackerT, CleanT> ap;
  ap(packer, val, 1);
}

template <typename T>
template <typename UnpackerT>
T* Dispatch<T>::unpackTypeWithUnpacker(
  UnpackerT& unpacker, SerialByteType* buf, bool in_place
) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;

  if (in_place) {
    auto t_buf = reinterpret_cast<T*>(buf);
    SerializerDispatch<Unpacker, CleanT> ap;
    ap(unpacker, t_buf, 1);
    return t_buf;
  } else {
    DeserializerDispatch<Serializer, CleanT> apply_des;
    auto& target = apply_des(unpacker,buf);
    SerializerDispatch<Unpacker, CleanT> ap;
    ap(unpacker, &target, 1);
    return &target;
  }
}

template <typename T>
template <typename PackerT, typename... Args>
PackerT Dispatch<T>::packType(
  T& target, SerialSizeType const& size, Args&&... args
) {
  PackerT packer(size, std::forward<Args>(args)...);
  packTypeWithPacker(packer, target, size);
  return packer;
}

template <typename T>
template <typename UnpackerT, typename... Args>
T* Dispatch<T>::unpackType(
  SerialByteType* buf, bool in_place, Args&&... args
) {
  UnpackerT unpacker(std::forward<Args>(args)...);
  return unpackTypeWithUnpacker(unpacker, buf, in_place);
}


template <typename T, typename BufferT, typename... Args>
T* unpack(SerialByteType* buf, bool in_place, Args&&... args) {
  using UnpackerType = UnpackerBuffer<BufferT>;
  auto p = Dispatch<T>::template unpackType<UnpackerType>(
    buf, in_place, std::forward<Args>(args)...
  );
  return p;
}

template <typename Serializer, typename T>
inline void serializeArray(Serializer& s, T* array, SerialSizeType const num_elms) {
  using DispatchT = DispatchCommon<T>;
  using CleanT = typename DispatchT::CleanT;
  auto val = DispatchT::clean(array);

  SerializerDispatch<Serializer, CleanT> ap;
  ap(s, val, num_elms);
}

template <typename T, typename BufferT, typename... Args>
PackerBuffer<BufferT> pack(
  T& target, SerialSizeType size, Args&&... args
) {
  using PackerType = PackerBuffer<BufferT>;
  auto p = Dispatch<T>::template packType<PackerType>(
    target, size, std::forward<Args>(args)...
  );
  return p;
}

template <typename T>
buffer::ImplReturnType packType(
  T& target, SerialSizeType size, BufferObtainFnType fn
) {
  SerialByteType* user_buf = fn ? fn(size) : nullptr;
  if (user_buf == nullptr) {
    auto p = pack<T, buffer::ManagedBuffer>(target, size);
    return std::make_tuple(std::move(p.extractPackedBuffer()), size);
  } else {
    auto p = pack<T, buffer::UserBuffer>(
      target, size, std::make_unique<buffer::UserBuffer>(user_buf, size)
    );
    return std::make_tuple(std::move(p.extractPackedBuffer()), size);
  }
}

template <typename T>
buffer::ImplReturnType serializeType(T& target, BufferObtainFnType fn) {
  auto size = Dispatch<T>::sizeType(target);
  debug_checkpoint("serializeType: size=%ld\n", size);
  return packType<T>(target, size, fn);
}

template <typename T>
T* deserializeType(SerialByteType* data, SerialByteType* allocBuf) {
  auto mem = allocBuf ? allocBuf : new SerialByteType[sizeof(T)];
  return unpack<T, buffer::UserBuffer>(mem, false, data);
}

template <typename T>
void deserializeType(InPlaceTag, SerialByteType* data, T* t) {
  auto t_place = reinterpret_cast<SerialByteType*>(t);
  unpack<T, buffer::UserBuffer>(t_place, true, data);
}

template <typename T>
std::size_t sizeType(T& target) {
  return Dispatch<T>::sizeType(target);
}

}} /* end namespace checkpoint::dispatch */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_IMPL_H*/
