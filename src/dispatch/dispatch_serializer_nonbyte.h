/*
//@HEADER
// ************************************************************************
//
//                          dispatch_serializer_nonbyte.h
//                           checkpoint
//              Copyright (C) 2017 NTESS, LLC
//
// Under the terms of Contract DE-NA-0003525 with NTESS, LLC,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// ************************************************************************
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
  void partial(SerializerT& s, T* val, SizeType num) {
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
    SerializerT& s, T* val, SizeType num,
    hasInParserdes<U>* __attribute__((unused)) x = nullptr
  ) {
    debug_serdes("SerializerDispatch: intrusive parserdes: val=%p\n", &val);
    for (SizeType i = 0; i < num; i++) {
      val[i].template parserdes<SerializerT>(s);
    }
  }

  #if HAS_DETECTION_COMPONENT
  template <typename U = T>
  void applyPartial(
    SerializerT& s, T* val, SizeType num,
    hasNonInParserdes<U>* __attribute__((unused)) x = nullptr
  ) {
    debug_serdes("SerializerDispatch: intrusive parserdes: val=%p\n", &val);
    for (SizeType i = 0; i < num; i++) {
      parserdes(s, val[i]);
    }
  }
  #endif

  template <typename U = T>
  void applyPartial(
    SerializerT& s, T* val, SizeType num,
    hasNoParserdes<U>* __attribute__((unused)) x = nullptr
  ) {
    // do nothing, it can be skipped
  }
};

template <typename SerializerT, typename T>
struct SerializerDispatchNonByte {

  template <typename U = T>
  void operator()(SerializerT& s, T* val, SizeType num) {
    return apply(s, val, num);
  }

  // If we have the detection component, we can more precisely check for
  // serializability
  #if HAS_DETECTION_COMPONENT
    template <typename U>
    using hasSplitSerialize =
    typename std::enable_if<SerializableTraits<U>::has_split_serialize, T>::type;

    template <typename U>
    using hasNotSplitSerialize =
    typename std::enable_if<!SerializableTraits<U>::has_split_serialize, T>::type;

    template <typename U>
    using hasInSerialize =
    typename std::enable_if<SerializableTraits<U>::has_serialize_instrusive, T>::type;

    template <typename U>
    using hasNoninSerialize =
    typename std::enable_if<SerializableTraits<U>::has_serialize_noninstrusive, T>::type;
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
    hasSplitSerialize<U>* __attribute__((unused)) x = nullptr
  ) {
    val->template serializeParent<SerializerT>(s);
    val->template serializeThis<SerializerT>(s);
  }

  template <typename U = T>
  void applyElm(
    SerializerT& s, T* val,
    hasNotSplitSerialize<U>* __attribute__((unused)) x = nullptr
  ) { }

  template <typename U = T>
  void apply(
    SerializerT& s, T* val, SizeType num,
    hasInSerialize<U>* __attribute__((unused)) x = nullptr
  ) {
    debug_serdes("SerializerDispatch: intrusive serialize: val=%p\n", &val);
    for (SizeType i = 0; i < num; i++) {
      val[i].template serialize<SerializerT>(s);
      applyElm(s, val+i);
    }
  }

  template <typename U = T>
  void apply(
    SerializerT& s, T* val, SizeType num,
    hasNoninSerialize<U>* __attribute__((unused)) x = nullptr
  ) {
    debug_serdes("SerializerDispatch: non-intrusive serialize: val=%p\n", &val);
    for (SizeType i = 0; i < num; i++) {
      serialize(s, val[i]);
    }
  }
};

} //end namespace serdes

#endif /*INCLUDED_SERDES_DISPATCH_SERIALIZER_NONBYTE*/
