/*
//@HEADER
// *****************************************************************************
//
//                               serializer_ref.h
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

#if !defined INCLUDED_SRC_CHECKPOINT_SERIALIZERS_SERIALIZER_REF_H
#define INCLUDED_SRC_CHECKPOINT_SERIALIZERS_SERIALIZER_REF_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/base_serializer.h"
#include "checkpoint/traits/user_traits.h"

#include <type_traits>
#include <cstdlib>
#include <memory>

namespace checkpoint {

namespace {
  // Cuda does not play nicely with templated static constexpr
  // member variables in the SerializerRef class, so we make a
  // helper struct to hold non-templated static constexpr members.
  template<bool B>
  struct bool_enable_if {
    static constexpr bool value = false;
  };

  template<>
  struct bool_enable_if<true> {
    static constexpr bool value = true;
    using type = void*;
  };
}

template<typename SerT, typename UserTraits = UserTraitHolder<>>
struct SerializerRef
{
  using TraitHolder = typename UserTraits::BaseTraits;

  //Passing initialization to the serializer implementation
  explicit SerializerRef(SerT* m_impl, const UserTraits& = {}) :
    impl(m_impl)
  { }

  //Copy constructor, possibly changing the traits
  template<typename OtherTraits>
  explicit SerializerRef(
    const SerializerRef<SerT, OtherTraits>& other
  ) : impl(other.impl)
  { }

  //Now forward along Serializer function calls and types
  using ModeType = eSerializationMode;
  template <typename SerializerT, typename T>
  using DispatcherType = typename SerT::template DispatcherType<SerializerT, T>;
  ModeType getMode() const { return impl->getMode(); }
  bool isSizing() const { return impl->isSizing(); }
  bool isPacking() const { return impl->isPacking(); }
  bool isUnpacking() const { return impl->isUnpacking(); }
  bool isFootprinting() const { return impl->isFootprinting(); }
  template<typename T>
  void countBytes(const T& t) { impl->countBytes(t); }
  void addBytes(std::size_t s) { impl->addBytes(s); }
  void contiguousBytes(void* ptr, SerialSizeType size, SerialSizeType num_elms) {
    impl->contiguousBytes(ptr, size, num_elms);
  }
  SerialSizeType usedBufferSize() const { return impl->usedBufferSize(); }
  template<typename SerializerT, typename T>
  void contiguousTyped(SerializerT& serdes, T* ptr, SerialSizeType num_elms) {
    serdes.contiguousBytes(static_cast<void*>(ptr), sizeof(T), num_elms);
  }
  template<typename... Args>
  void skip(Args&&... args){
    impl->skip( std::forward<Args>(args)... );
  }
  SerialByteType* getBuffer() const { return impl->getBuffer(); }
  SerialByteType* getSpotIncrement(const SerialSizeType inc) { return impl->getSpotIncrement(inc); }
  bool isVirtualDisabled() const { return impl->isVirtualDisabled(); }
  void setVirtualDisabled(bool val) { impl->setVirtualDisabled(val); }


  SerT* operator->(){ return impl; }

  template<typename OtherTraits>
  operator SerializerRef<SerT, OtherTraits>() const {
    return SerializerRef<SerT, OtherTraits>(*this);
  }

  //Big block of helpers for conveniently checking traits in different contexts.
  template<typename... Traits>
  using has_traits = bool_enable_if<TraitHolder::template has<Traits...>::value>;
  template<typename... Traits>
  using has_any_traits = bool_enable_if<TraitHolder::template has_any<Traits...>::value>;

  template<typename... Traits>
  using has_not_traits = bool_enable_if<!(has_traits<Traits...>::value)>;
  template<typename... Traits>
  using has_not_any_traits = bool_enable_if<!(has_any_traits<Traits...>::value)>;

  //Helpers for converting between traits
  using TraitlessT = SerializerRef<SerT>;

  //Returns a new reference with traits in addition to this reference's traits.
  template<typename Trait, typename... Traits>
  auto withTraits(UserTraitHolder<Trait, Traits...> = {}){
    using NewTraitHolder = typename TraitHolder::template with<Trait, Traits...>;
    return SerializerRef<SerT, NewTraitHolder>(*this);
  }

  //Returns a new reference with traits removed (if present) from this reference's traits.
  template<typename Trait, typename... Traits>
  auto withoutTraits(UserTraitHolder<Trait, Traits...> = {}){
    using NewTraitHolder = typename TraitHolder::template without<Trait, Traits...>;
    return SerializerRef<SerT, NewTraitHolder>(*this);
  }

  //Returns a new reference with traits set to the inputs, regardless of this reference's traits.
  template<typename... NewTraits>
  SerializerRef<SerT, UserTraitHolder<NewTraits...>> setTraits(
      const UserTraitHolder<NewTraits...>& = {}
  ){
    return SerializerRef<SerT, UserTraitHolder<NewTraits...>>(*this);
  }

protected:
  SerT *const impl;

  template<typename, typename>
  friend struct SerializerRef;
};

}
#endif /*INCLUDED_SRC_CHECKPOINT_SERIALIZERS_SERIALIZER_REF_H*/
