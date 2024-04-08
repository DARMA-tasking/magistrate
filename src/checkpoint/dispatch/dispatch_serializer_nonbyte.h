/*
//@HEADER
// *****************************************************************************
//
//                        dispatch_serializer_nonbyte.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_SERIALIZER_NONBYTE_H
#define INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_SERIALIZER_NONBYTE_H

#include "checkpoint/common.h"
#include "checkpoint/traits/serializable_traits.h"
#include "checkpoint/dispatch/vrt/virtual_serialize_traits.h"
#include "checkpoint/dispatch/vrt/virtual_serialize.h"

#include <type_traits>
#include <tuple>
#include <cstdlib>
#include <cassert>

namespace checkpoint { namespace dispatch {

template <typename SerializerT, typename T>
struct BasicDispatcher {
  static void serializeIntrusive(SerializerT& s, T& t) {
    t.serialize(s);
  }

  static void serializeNonIntrusive(SerializerT& s, T& t) {
    serialize(s, t);
  }
};

template <
  typename SerializerT,
  typename T,
  typename Dispatcher = BasicDispatcher<SerializerT, T>
>
struct SerializerDispatchNonByte {
  using S = SerializerT;


  template <typename U = T>
  void operator()(SerializerT& s, T* val, SerialSizeType num) {
    apply(s, val, num);
  }

  template <typename U>
  using hasSplitSerialize =
  typename std::enable_if<SerializableTraits<U,S>::has_split_serialize, T>::type;

  template <typename U>
  using hasNotSplitSerialize =
  typename std::enable_if<!SerializableTraits<U,S>::has_split_serialize && !std::is_enum<U>::value, T>::type;

  template <typename U>
  using hasInSerialize =
  typename std::enable_if<SerializableTraits<U,S>::has_serialize_instrusive, T>::type;

  template <typename U>
  using hasNoninSerialize =
  typename std::enable_if<SerializableTraits<U,S>::has_serialize_noninstrusive, T>::type;

  template <typename U>
  using hasVirtualSerialize =
  typename std::enable_if<vrt::VirtualSerializeTraits<U>::has_virtual_serialize, T>::type;

  template <typename U>
  using hasNotVirtualSerialize =
  typename std::enable_if<vrt::VirtualSerializeTraits<U>::has_not_virtual_serialize, T>::type;

  template <typename U>
  using isEnum =
  typename std::enable_if<std::is_enum<U>::value, T>::type;

  template <typename U>
  using justFootprint =
  typename std::enable_if<
    std::is_same<S, checkpoint::Footprinter>::value and
    not SerializableTraits<U, S>::is_traversable and
    not std::is_enum<U>::value,
    T
  >::type;

  template <typename U = T>
  void applyElm(
    SerializerT& s, T* val, hasSplitSerialize<U>* = nullptr
  ) {
    val->template serializeParent<SerializerT>(s);
    val->template serializeThis<SerializerT>(s);
  }

  template <typename U = T>
  void applyElm(
    SerializerT&, T*, hasNotSplitSerialize<U>* = nullptr
  ) { }

  template <typename U = T>
  void applyElm(
    SerializerT& s, T* val, isEnum<U>* = nullptr
  ) {
    serializeEnum(s, *val);
  }

  /**
   * \brief Dispatch non-serializable types when footprinting
   *
   * Allow simplified footprinting of classes that do not support serialization
   * by just applying 'sizeof' operator on them.
   * Note: this means that complex objects will not be traversed further.
   *
   * \param[in] s serializer to use
   * \param[in] val pointer to the array of objects
   * \param[in] num number of objects in the array
   */
  template <typename U = T>
  void applyStatic(
    SerializerT& s, T* val, SerialSizeType num, justFootprint<U>* = nullptr
  ) {
    debug_checkpoint(
      "SerializerDispatch: justFootprint: val=%p typeid=%s\n",
      static_cast<void*>(&val),
      typeid(val).name()
    );
    static bool firstCall = true;
    if (firstCall) {
      firstCall = false;
      debug_checkpoint(
        "SerializerDispatch: warning: simplified footprinting in use!\n"
        "Objects of type: typeid=%s will not be traversed\n",
        typeid(val).name()
      );
    }

    s.contiguousBytes(val, sizeof(T), num);
  }

  template <typename U = T>
  void applyStatic(
    SerializerT& s, T* val, SerialSizeType num, hasInSerialize<U>* = nullptr
  ) {
    debug_checkpoint(
      "SerializerDispatch: intrusive serialize: val=%p typeid=%s\n",
      static_cast<void*>(&val),
      typeid(val).name()
    );
    for (SerialSizeType i = 0; i < num; i++) {
      Dispatcher::serializeIntrusive(s, val[i]);
      applyElm(s, val+i);
    }
  }

  template <typename U = T>
  void applyStatic(
    SerializerT& s, T* val, SerialSizeType num, hasNoninSerialize<U>* = nullptr
  ) {
    debug_checkpoint(
      "SerializerDispatch: non-intrusive serialize: val=%p typeid=%s\n",
      static_cast<void*>(&val),
      typeid(val).name()
    );
    for (SerialSizeType i = 0; i < num; i++) {
      Dispatcher::serializeNonIntrusive(s, val[i]);
    }
  }

  template <typename U = T>
  void applyStatic(
    SerializerT& s, T* val, SerialSizeType num, isEnum<U>* = nullptr
  ) {
    debug_checkpoint(
      "SerializerDispatch: enum serialize: val=%p typeid=%s\n",
      static_cast<void*>(&val),
      typeid(val).name()
    );
    for (SerialSizeType i = 0; i < num; i++) {
      applyElm(s, val+i);
    }
  }

  template <typename U = T>
  void apply(
    SerializerT& s, T* val, SerialSizeType num, hasVirtualSerialize<U>* = nullptr
  ) {
    if (s.isVirtualDisabled()) {
      s.setVirtualDisabled(false);
      applyStatic(s, val, num);
    } else {
      dispatch::vrt::virtualSerialize(val, s);
    }
  }

  template <typename U = T>
  void apply(
    SerializerT& s, T* val, SerialSizeType num, hasNotVirtualSerialize<U>* = nullptr
  ) {
    applyStatic(s, val, num);
  }
};

}} /* end namespace checkpoint::dispatch */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_SERIALIZER_NONBYTE_H*/
