/*
//@HEADER
// *****************************************************************************
//
//                               dispatch.impl.h
//                 DARMA/magistrate => Serialization Library
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

#if !defined INCLUDED_SRC_CHECKPOINT_DISPATCH_DISPATCH_IMPL_H
#define INCLUDED_SRC_CHECKPOINT_DISPATCH_DISPATCH_IMPL_H

#include "checkpoint/common.h"
#include "checkpoint/dispatch/dispatch.h"
#include "checkpoint/dispatch/type_registry.h"

#include <stdexcept>
#include <string>

namespace checkpoint {

template <typename Serializer, typename T>
inline Serializer& operator|(Serializer& s, T& target) {
  return dispatch::Traverse::with(target, s);
}

} /* end namespace checkpoint */

namespace checkpoint { namespace dispatch {

struct serialization_error : public std::runtime_error {
  explicit serialization_error(std::string const& msg, int const depth = 0)
    : std::runtime_error(msg),
      depth_(depth) { }

  int const depth_ = 0;
};

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
      auto const err = std::string("Unpacking wrong type, got=") +
        typeregistry::getTypeNameForIdx(thisTypeIdx) +
        " (idx=" + std::to_string(thisTypeIdx) +
        "), expected=" + typeregistry::getTypeNameForIdx(serTypeIdx) +
        " (idx=" + std::to_string(serTypeIdx) + ")\n#0 " +
        typeregistry::getTypeName<T>();
      throw serialization_error(err);
    }
  }

  return t;
}

template <typename T, typename TraverserT>
TraverserT& withMemUsed(TraverserT& t, SerialSizeType len) {
  using DispatchType =
    typename TraverserT::template DispatcherType<TraverserT, SerialSizeType>;
  SerializerDispatch<TraverserT, SerialSizeType, DispatchType> ap;

  SerialSizeType const memUsed = sizeof(T) * len;
  constexpr SerialSizeType memUsedLen = 1;
  if (t.isPacking() || t.isSizing()) {
    auto val = cleanType(&memUsed);
    ap(t, val, memUsedLen);
  } else if (t.isUnpacking()) {
    SerialSizeType serMemUsed = 0;
    auto val = cleanType(&serMemUsed);
    ap(t, val, memUsedLen);

    if (memUsed != serMemUsed) {
      using CleanT = typename CleanType<T>::CleanT;
      std::string msg = "For type '" + typeregistry::getTypeName<CleanT>() +
        "' serialization used " + std::to_string(serMemUsed) +
        "B, but deserialization used " + std::to_string(memUsed) + "B";

      throw serialization_error(msg);
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
  } catch (serialization_error const& err) {
    auto const depth = err.depth_ + 1;
    auto const what = std::string(err.what()) + "\n#" + std::to_string(depth) +
      " " + typeregistry::getTypeName<T>();
    throw serialization_error(what, depth);
  }
  #else
  ap(t, val, len);
  #endif

  #if defined(SERIALIZATION_ERROR_CHECKING)
  withMemUsed<CleanT>(t, len);
  #endif

  return t;
}

template <typename T, typename TraverserT, typename... Args>
TraverserT Traverse::with(T& target, Args&&... args) {
  #if !defined(SERIALIZATION_ERROR_CHECKING)
  using CleanT = typename CleanType<T>::CleanT;
  #endif

  TraverserT t(std::forward<Args>(args)...);

  #if !defined(SERIALIZATION_ERROR_CHECKING)
  withTypeIdx<CleanT>(t);
  #endif

  with(target, t);

  #if !defined(SERIALIZATION_ERROR_CHECKING)
  withMemUsed<CleanT>(t, 1);
  #endif

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

template <typename TargetT, typename PackerT>
inline void
validatePackerBufferSize(PackerT const& p, SerialSizeType bufferSize) {
  auto const usedBufferSize = p.usedBufferSize();
  if (usedBufferSize != bufferSize) {
    using CleanT = typename CleanType<TargetT>::CleanT;

    std::string msg = "For type '" + typeregistry::getTypeName<CleanT>() +
      "' Sizer reported " + std::to_string(bufferSize) + "B, but Packer used " +
      std::to_string(usedBufferSize) + "B";
    throw serialization_error(msg);
  }
}

template <typename T>
buffer::ImplReturnType
packBuffer(T& target, SerialSizeType size, BufferObtainFnType fn) {
  SerialByteType* user_buf = fn ? fn(size) : nullptr;
  if (user_buf == nullptr) {
    auto p =
      Standard::pack<T, PackerBuffer<buffer::ManagedBuffer>>(target, size);
    validatePackerBufferSize<T>(p, size);
    return std::make_tuple(std::move(p.extractPackedBuffer()), size);
  } else {
    auto p = Standard::pack<T, PackerBuffer<buffer::UserBuffer>>(
      target, size, std::make_unique<buffer::UserBuffer>(user_buf, size)
    );
    validatePackerBufferSize<T>(p, size);
    return std::make_tuple(std::move(p.extractPackedBuffer()), size);
  }
}

template <typename T>
typename std::enable_if<
  !vrt::VirtualSerializeTraits<T>::has_virtual_serialize,
  buffer::ImplReturnType>::type
serializeType(T& target, BufferObtainFnType fn) {
  auto len = Standard::size<T, Sizer>(target);
  debug_checkpoint("serializeType: len=%ld\n", len);
  return packBuffer<T>(target, len, fn);
}

template <typename T>
typename std::enable_if<!vrt::VirtualSerializeTraits<T>::has_virtual_serialize, T*>::type
deserializeType(SerialByteType* data, SerialByteType* allocBuf) {
  auto mem = allocBuf ? allocBuf : Standard::allocate<T>();
  auto t_buf = std::unique_ptr<T>(Standard::construct<T>(mem));
  T* traverser =
    Standard::unpack<T, UnpackerBuffer<buffer::UserBuffer>>(t_buf.get(), data);
  t_buf.release();
  return traverser;
}

template <typename T>
void deserializeType(InPlaceTag, SerialByteType* data, T* t) {
  Standard::unpack<T, UnpackerBuffer<buffer::UserBuffer>>(t, data);
}

template <typename T>
struct PrefixedType {
  using BaseType = vrt::checkpoint_base_type_t<T>;

  // Create PrefixedType for serialization purposes
  explicit PrefixedType(BaseType* target) : target_(target) {
    prefix_ = target->_checkpointDynamicTypeIndex();
  }

  // Create PrefixedType for deserialization purposes
  explicit PrefixedType(SerialByteType* allocBuf) : unpack_buf_(allocBuf) { }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | prefix_;

    // Determine the correct type and allocate memory
    if (s.isUnpacking()) {
      validatePrefix(prefix_);

      auto mem = unpack_buf_ ? unpack_buf_ : vrt::objregistry::allocateConcreteType<BaseType>(prefix_);
      target_ = vrt::objregistry::constructConcreteType<BaseType>(prefix_, mem);
    }

    s | *target_;
  }

  BaseType* getTarget() const {
    return target_;
  }

private:
  void validatePrefix(vrt::TypeIdx prefix) {
    if (!vrt::objregistry::isValidIdx<BaseType>(prefix)) {
      std::string const err = std::string("Unpacking invalid prefix type (") +
        std::to_string(prefix) + std::string(") from object registry for type=") +
        std::string(typeregistry::getTypeName<BaseType>());
      throw serialization_error(err);
    }
  }

  vrt::TypeIdx prefix_ = 0;
  BaseType* target_ = nullptr;
  SerialByteType* unpack_buf_ = nullptr;
};

template <typename T>
typename std::enable_if<
  vrt::VirtualSerializeTraits<T>::has_virtual_serialize,
  buffer::ImplReturnType>::type
serializeType(T& target, BufferObtainFnType fn) {
  using BaseType = vrt::checkpoint_base_type_t<T>;
  using PrefixedType = PrefixedType<BaseType>;

  auto prefixed = PrefixedType(&target);
  auto len = Standard::size<PrefixedType, Sizer>(prefixed);
  debug_checkpoint("serializeType: len=%ld\n", len);
  return packBuffer<PrefixedType>(prefixed, len, fn);
}

template <typename T>
typename std::enable_if<vrt::VirtualSerializeTraits<T>::has_virtual_serialize, T*>::type
deserializeType(SerialByteType* data, SerialByteType* allocBuf) {
  using BaseType = vrt::checkpoint_base_type_t<T>;
  using PrefixedType = PrefixedType<BaseType>;

  auto prefixed = PrefixedType(allocBuf);
  auto* traverser = Standard::unpack<PrefixedType, UnpackerBuffer<buffer::UserBuffer>>(&prefixed, data);
  return static_cast<T*>(traverser->getTarget());
}

}} /* end namespace checkpoint::dispatch */

#endif /*INCLUDED_SRC_CHECKPOINT_DISPATCH_DISPATCH_IMPL_H*/
