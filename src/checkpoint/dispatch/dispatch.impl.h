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
#include "checkpoint/dispatch/type_registry.h"

#include <stdexcept>
#include <string>
#include <type_traits>

namespace checkpoint {

template <typename Serializer, typename T>
inline Serializer& operator|(Serializer& s, T& target) {
  return dispatch::Traverse::with(target, s);
}

} /* end namespace checkpoint */

namespace checkpoint { namespace dispatch {

template <typename T, typename TraverserT>
TraverserT& withTypeIdx(TraverserT& t) {
  using CleanT = typename CleanType<typeregistry::DecodedIndex>::CleanT;
  using DispatchType =
    typename TraverserT::template DispatcherType<TraverserT, CleanT>;

  auto const thisTypeIdx = typeregistry::getTypeIdx<T>();
  SerializerDispatch<TraverserT, CleanT, DispatchType> ap;
  constexpr SerialSizeType len = 1;
  if (t.isPacking() || t.isSizing()) {
    auto val = cleanType(&thisTypeIdx);
    ap(t, val, len);
  } else if (t.isUnpacking()) {
    typeregistry::DecodedIndex serTypeIdx = 0;
    auto val = cleanType(&serTypeIdx);
    ap(t, val, len);

    if (
      typeregistry::validateIndex(serTypeIdx) == false ||
      thisTypeIdx != serTypeIdx
    ) {
      auto err = std::string("Unpacking wrong type, got=") +
        typeregistry::getTypeNameForIdx(thisTypeIdx) +
        " idx=" + std::to_string(thisTypeIdx) +
        ", expected=" + typeregistry::getTypeNameForIdx(serTypeIdx) +
        " idx=" + std::to_string(serTypeIdx);
      throw std::runtime_error(err);
    }
  }

  return t;
}

template <typename T, typename TraverserT>
TraverserT& Traverse::with(T& target, TraverserT& t, SerialSizeType len) {
  using CleanT = typename CleanType<T>::CleanT;
  using DispatchType =
    typename TraverserT::template DispatcherType<TraverserT, CleanT>;

  #if defined(SERIALIZATION_ERROR_CHECKING)
  withTypeIdx<CleanT>(t);
  #endif

  auto val = cleanType(&target);
  SerializerDispatch<TraverserT, CleanT, DispatchType> ap;

  #if defined(SERIALIZATION_ERROR_CHECKING)
  try {
    ap(t, val, len);
  } catch (std::runtime_error& err) {
    auto const what =
      std::string{err.what()} + "\npath: " + typeregistry::getTypeName<T>();
    throw std::runtime_error(what);
  }
  #else
  ap(t, val, len);
  #endif

  return t;
}

template <typename T, typename TraverserT, typename... Args>
TraverserT Traverse::with(T& target, Args&&... args) {
  TraverserT t(std::forward<Args>(args)...);

  #if !defined(SERIALIZATION_ERROR_CHECKING)
  using CleanT = typename CleanType<T>::CleanT;
  withTypeIdx<CleanT>(t);
  #endif

  with(target, t);
  return t;
}

template <typename T>
T* Traverse::reconstruct(SerialByteType* mem) {
  return Reconstructor<typename CleanType<T>::CleanT>::construct(mem);
}

template <typename T, typename SizerT, typename... Args>
SerialSizeType Standard::size(T& target, Args&&... args) {
  auto sizer = Traverse::with<T, SizerT>(target, std::forward<Args>(args)...);
  return sizer.getSize();
}

template <typename T, typename FootprinterT, typename... Args>
SerialSizeType Standard::footprint(T& target, Args&&... args) {
  auto footprinter =
    Traverse::with<T, FootprinterT>(target, std::forward<Args>(args)...);
  return footprinter.getMemoryFootprint();
}

template <typename T, typename PackerT, typename... Args>
PackerT Standard::pack(T& target, SerialSizeType const& size, Args&&... args) {
  return Traverse::with<T, PackerT>(target, size, std::forward<Args>(args)...);
}

template <typename T>
SerialByteType* Standard::allocate() {
  return reinterpret_cast<SerialByteType*>(std::allocator<T>{}.allocate(1));
}

template <typename T, typename UnpackerT, typename... Args>
T* Standard::unpack(T* t_buf, Args&&... args) {
  Traverse::with<T, UnpackerT>(*t_buf, std::forward<Args>(args)...);
  return t_buf;
}

template <typename T>
T* Standard::construct(SerialByteType* mem) {
  return Traverse::reconstruct<T>(mem);
}

template <typename Serializer, typename T>
inline void serializeArray(Serializer& s, T* array, SerialSizeType const len) {
  if (len > 0) {
    Traverse::with<T, Serializer>(*array, s, len);
  }
}

template <typename T>
buffer::ImplReturnType
packBuffer(T& target, SerialSizeType size, BufferObtainFnType fn) {
  SerialByteType* user_buf = fn ? fn(size) : nullptr;
  if (user_buf == nullptr) {
    auto p =
      Standard::pack<T, PackerBuffer<buffer::ManagedBuffer>>(target, size);
    return std::make_tuple(std::move(p.extractPackedBuffer()), size);
  } else {
    auto p = Standard::pack<T, PackerBuffer<buffer::UserBuffer>>(
      target, size, std::make_unique<buffer::UserBuffer>(user_buf, size));
    return std::make_tuple(std::move(p.extractPackedBuffer()), size);
  }
}

template <typename T>
buffer::ImplReturnType serializeType(T& target, BufferObtainFnType fn) {
  auto len = Standard::size<T, Sizer>(target);
  debug_checkpoint("serializeType: len=%ld\n", len);
  return packBuffer<T>(target, len, fn);
}

template <typename T>
T* deserializeType(SerialByteType* data, SerialByteType* allocBuf) {
  auto mem = allocBuf ? allocBuf : Standard::allocate<T>();
  T* t_buf = Standard::construct<T>(mem);
  return Standard::unpack<T, UnpackerBuffer<buffer::UserBuffer>>(t_buf, data);
}

template <typename T>
void deserializeType(InPlaceTag, SerialByteType* data, T* t) {
  Standard::unpack<T, UnpackerBuffer<buffer::UserBuffer>>(t, data);
}

}} /* end namespace checkpoint::dispatch */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_IMPL_H*/
