/*
//@HEADER
// *****************************************************************************
//
//                        dispatch_serializer_nonbyte.h
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

#if ! defined INCLUDED_SERDES_DISPATCH_SERIALIZER_NONBYTE
#define INCLUDED_SERDES_DISPATCH_SERIALIZER_NONBYTE

#include "serdes_common.h"
#include "serdes_all.h"
#include "traits/serializable_traits.h"

#include <type_traits>
#include <tuple>
#include <cstdlib>
#include <cassert>

namespace serdes {

// Implement a lightweight simple detection mechanism (when full detector is not
// present) to detect type of serialize() method: intrusive/non-intrusive
// variants
#if !HAS_DETECTION_COMPONENT
template <typename SerializerT, typename T>
struct hasSerialize {
  template <
    typename C,
    typename = decltype(std::declval<C>().serialize(std::declval<SerializerT&>()))
  >
  static std::true_type test(int);

  template <typename C>
  static std::false_type test(...);

  static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename SerializerT, typename T>
struct hasParserdes {
  template <
    typename C,
    typename = decltype(std::declval<C>().parserdes(std::declval<SerializerT&>()))
  >
  static std::true_type test(int);

  template <typename C>
  static std::false_type test(...);

  static constexpr bool value = decltype(test<T>(0))::value;
};
#endif

template <typename SerializerT, typename T>
struct SerializerDispatchNonByteParserdes {
  template <typename U = T>
  void partial(SerializerT& s, T* val, SerialSizeType num) {
    return applyPartial(s, val, num);
  }

  // If we have the detection component, we can more precisely check for
  // serializability
  #if HAS_DETECTION_COMPONENT
    template <typename U>
    using hasInParserdes =
    typename std::enable_if<SerializableTraits<U>::has_int_parserdes, T>::type;

    template <typename U>
    using hasNonInParserdes =
    typename std::enable_if<SerializableTraits<U>::has_nonint_parserdes, T>::type;

    template <typename U>
    using hasNoParserdes =
    typename std::enable_if<!SerializableTraits<U>::has_parserdes, T>::type;
  #else
    template <typename U>
    using hasInParserdes =
    typename std::enable_if<hasParserdes<SerializerT, U>::value, T>::type;

    template <typename U>
    using hasNoParserdes =
    typename std::enable_if<!hasParserdes<SerializerT, U>::value, T>::type;
  #endif

  template <typename U = T>
  void applyPartial(
    SerializerT& s, T* val, SerialSizeType num,
    hasInParserdes<U>* __attribute__((unused)) x = nullptr
  ) {
    debug_serdes("SerializerDispatch: intrusive parserdes: val=%p\n", &val);
    for (SerialSizeType i = 0; i < num; i++) {
      val[i].template parserdes<SerializerT>(s);
    }
  }

  #if HAS_DETECTION_COMPONENT
  template <typename U = T>
  void applyPartial(
    SerializerT& s, T* val, SerialSizeType num,
    hasNonInParserdes<U>* __attribute__((unused)) x = nullptr
  ) {
    debug_serdes("SerializerDispatch: intrusive parserdes: val=%p\n", &val);
    for (SerialSizeType i = 0; i < num; i++) {
      parserdes(s, val[i]);
    }
  }
  #endif

  template <typename U = T>
  void applyPartial(
    SerializerT& s, T* val, SerialSizeType num,
    hasNoParserdes<U>* __attribute__((unused)) x = nullptr
  ) {
    // do nothing, it can be skipped
  }
};

template <typename SerializerT, typename T>
struct SerializerDispatchNonByte {

  template <typename U = T>
  void operator()(SerializerT& s, T* val, SerialSizeType num) {
    return apply(s, val, num);
  }

  // If we have the detection component, we can more precisely check for
  // serializability
  #if HAS_DETECTION_COMPONENT
    template <typename U>
    using hasInSerialize =
    typename std::enable_if<SerializableTraits<U>::has_serialize_instrusive, T>::type;

    template <typename U>
    using hasNoninSerialize =
    typename std::enable_if<SerializableTraits<U>::has_serialize_noninstrusive, T>::type;

    template <typename U>
    using isEnum =
    typename std::enable_if<std::is_enum<U>::value, T>::type;
  #else
    template <typename U>
    using hasInSerialize =
    typename std::enable_if<hasSerialize<SerializerT, U>::value, T>::type;

    template <typename U>
    using hasNoninSerialize =
    typename std::enable_if<!hasSerialize<SerializerT, U>::value, T>::type;
  #endif

  template <typename U = T>
  void applyElm(
    SerializerT& s, T* val,
    isEnum<U>* __attribute__((unused)) x = nullptr
  ) {
    serializeEnum(s, *val);
  }

  template <typename U = T>
  void apply(
    SerializerT& s, T* val, SerialSizeType num,
    hasInSerialize<U>* __attribute__((unused)) x = nullptr
  ) {
    debug_serdes("SerializerDispatch: intrusive serialize: val=%p\n", &val);
    for (SerialSizeType i = 0; i < num; i++) {
      val[i].template _serdes_internal_serialize<SerializerT, T>(s); // Instantiation error here? Inherit from serdes::base or through serdes::inherit
    }
  }

  template <typename U = T>
  void apply(
    SerializerT& s, T* val, SerialSizeType num,
    hasNoninSerialize<U>* __attribute__((unused)) x = nullptr
  ) {
    debug_serdes("SerializerDispatch: non-intrusive serialize: val=%p\n", &val);
    for (SerialSizeType i = 0; i < num; i++) {
      serialize(s, val[i]);
    }
  }

  template <typename U = T>
  void apply(
    SerializerT& s, T* val, SerialSizeType num,
    isEnum<U>* __attribute__((unused)) x = nullptr
  ) {
    debug_serdes("SerializerDispatch: enum serialize: val=%p\n", &val);
    for (SerialSizeType i = 0; i < num; i++) {
      applyElm(s, val+i);
    }
  }
};

} //end namespace serdes

#endif /*INCLUDED_SERDES_DISPATCH_SERIALIZER_NONBYTE*/
