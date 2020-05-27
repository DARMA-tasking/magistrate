/*
//@HEADER
// *****************************************************************************
//
//                             virtual_serialize.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_VRT_VIRTUAL_SERIALIZE_H
#define INCLUDED_CHECKPOINT_DISPATCH_VRT_VIRTUAL_SERIALIZE_H

#include "checkpoint/dispatch/vrt/base.h"
#include "checkpoint/dispatch/vrt/derived.h"

namespace checkpoint { namespace dispatch { namespace vrt {

struct OptionalTypeToken {
  OptionalTypeToken() = default;

  explicit OptionalTypeToken(bool in_is_virtual, TypeIdx in_type_idx = -1)
    : is_virtual_(in_is_virtual),
      type_idx_(in_type_idx)
  { }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | is_virtual_;
    if (is_virtual_) {
      s | type_idx_;
    }
  }

  bool isVirtual() const { return is_virtual_; }

  TypeIdx getTypeIdx() const {
    assert(not is_virtual_ or type_idx_ != -1 && "Must be valid type idx if virtual");
    return type_idx_;
  }

private:
  bool is_virtual_ = false;
  TypeIdx type_idx_ = -1;
};

template <typename T, typename _enabled = void>
struct RegisterIfNeeded;

template <typename T>
struct RegisterIfNeeded<
  T,
  typename std::enable_if_t<VirtualSerializeTraits<T>::has_virtual_serialize>
> {
  static TypeIdx apply(T* t) {
    return t->_checkpointDynamicTypeIndex();
  }
};

template <typename T>
struct RegisterIfNeeded<
  T,
  typename std::enable_if_t<VirtualSerializeTraits<T>::has_not_virtual_serialize>
> {
  static TypeIdx apply(T*) {
    return -1;
  }
};

template <typename T, typename SerializerT>
void serializeAllocatePointer(T*& object, SerializerT& s) {
  TypeIdx entry = -1;

  if (not s.isUnpacking()) {
    entry = RegisterIfNeeded<T>::apply(object);
  }

  OptionalTypeToken token(VirtualSerializeTraits<T>::has_virtual_serialize, entry);
  s | token;

  if (s.isUnpacking()) {
    if (token.isVirtual()) {
      // use type idx here, registration needed for proper type re-construction
      auto entry = token.getTypeIdx();
      auto t = objregistry::getObjAllocate<T>(entry)();
      object = objregistry::getObjConstruct<T>(entry)(t);
    } else {
      // no type idx needed in this case, static construction in default case
      auto t = std::allocator<T>{}.allocate(1);
      object = dispatch::Reconstructor<T>::construct(t);
    }
  }
}

/**
 * \brief A function to handle serialization of objects of a mix of
 * types in a virtual inheritance hierarchy
 *
 * This will automatically record the exact derived type at
 * serialization, and reconstruct objects accordingly at
 * deserialization.
 */
template <typename BaseT, typename SerializerT>
void virtualSerialize(BaseT*& base, SerializerT& s) {
  auto serializer_idx = serializer_registry::makeObjIdx<BaseT, SerializerT>();
  base->_checkpointDynamicSerialize(&s, serializer_idx, -1);
}

}}} /* end namespace checkpoint::dispatch::vrt */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_VRT_VIRTUAL_SERIALIZE_H*/
