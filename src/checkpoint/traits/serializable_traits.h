/*
//@HEADER
// *****************************************************************************
//
//                            serializable_traits.h
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

#if !defined INCLUDED_CHECKPOINT_TRAITS_SERIALIZABLE_TRAITS_H
#define INCLUDED_CHECKPOINT_TRAITS_SERIALIZABLE_TRAITS_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/serializers_headers.h"
#include "checkpoint/dispatch/reconstructor_tag.h"

#include <cstdint>
#include <cassert>

#if HAS_DETECTION_COMPONENT
#include "detector_headers.h"
#endif  /*HAS_DETECTION_COMPONENT*/

#if HAS_DETECTION_COMPONENT

/*
 * Start the traits class with all the archetypes that can be detected for
 * serialization
 */

namespace checkpoint {

template <typename T>
void reconstruct(T) { assert(0); }

struct SerdesByteCopy {
  using isByteCopyable = std::true_type;
};

namespace detail {
template <typename T>
struct isByteCopyableImpl {
  template <typename U>
  using byteCopyTrait_t = typename U::isByteCopyable;
  using has_byteCopyTraitTrue = detection::is_detected_convertible<std::true_type, byteCopyTrait_t, T>;
};
}

template <typename T>
struct isByteCopyable : detail::isByteCopyableImpl< T >::has_byteCopyTraitTrue {};

template <typename T>
struct SerializableTraits {
  template <typename U>
  using serialize_t = decltype(
    std::declval<U>().serialize(std::declval<Serializer&>())
  );
  using has_serialize = detection::is_detected<serialize_t, T>;

  template <typename U>
  using serializeParent_t =
    decltype(std::declval<U>().serializeParent(std::declval<Serializer&>()));
  using has_serializeParent = detection::is_detected<serializeParent_t, T>;

  template <typename U>
  using serializeThis_t =
    decltype(std::declval<U>().serializeThis(std::declval<Serializer&>()));
  using has_serializeThis = detection::is_detected<serializeThis_t, T>;

  using has_byteCopyTraitTrue = isByteCopyable< T >;

  template <typename U>
  using has_isArith = std::is_arithmetic<U>;

  template <typename U>
  using nonintrustive_serialize_t = decltype(serialize(
    std::declval<Serializer&>(), std::declval<U&>()
  ));
  using has_nonintrustive_serialize
  = detection::is_detected<nonintrustive_serialize_t, T>;

  template <typename U>
  using constructor_t = decltype(U());
  using has_default_constructor = detection::is_detected<constructor_t, T>;

  template <typename U>
  using tagged_constructor_t = decltype(U(std::declval<SERIALIZE_CONSTRUCT_TAG>()));
  using has_tagged_constructor = detection::is_detected<tagged_constructor_t, T>;

  template <typename U>
  using reconstruct_t = decltype(U::reconstruct(std::declval<void*>()));
  using has_reconstruct =
    detection::is_detected_convertible<T&, reconstruct_t, T>;

  template <typename U>
  using nonintrustive_reconstruct_t = decltype(
    reconstruct(
      std::declval<U*&>(),
      std::declval<void*>()
    )
  );
  //

  using has_nonintrusive_reconstruct =
    detection::is_detected<nonintrustive_reconstruct_t, T>;

  // This defines what it means to have parent serializability
  static constexpr auto const has_parent_serialize = has_serializeParent::value;

  // This defines what it means to have this serializability
  static constexpr auto const has_this_serialize = has_serializeThis::value;

  // This defines what it means to have this serializability
  static constexpr auto const has_split_serialize =
    has_parent_serialize and has_this_serialize;

  // This defines what it means to be reconstructible
  static constexpr auto const is_bytecopyable =
    has_byteCopyTraitTrue::value or has_isArith<T>::value;

  // This defines what it means to be reconstructible
  static constexpr auto const is_reconstructible =
    has_reconstruct::value and not has_default_constructor::value;

  // This defines what it means to be non-intrusively reconstructible
  static constexpr auto const is_nonintrusive_reconstructible =
    has_nonintrusive_reconstruct::value and
    not has_default_constructor::value and
    not has_reconstruct::value;

  // This defines what it means to be default constructible
  static constexpr auto const is_default_constructible =
    has_default_constructor::value;

  // If the class has tagged constructor especially for constructing
  static constexpr auto const is_tagged_constructible =
    has_tagged_constructor::value;

  static constexpr auto const has_serialize_instrusive =
    has_serialize::value;
  static constexpr auto const has_serialize_noninstrusive =
    has_nonintrustive_serialize::value;

  // This defines what it means to have a serialize function
  static constexpr auto const has_serialize_function =
    has_serialize_instrusive or has_serialize_noninstrusive;

  // This defines what it means to be serializable
  static constexpr auto const is_serializable =
    has_serialize_function and (is_default_constructible or is_reconstructible);
};

}  // end namespace checkpoint

#endif  /*HAS_DETECTION_COMPONENT*/

#endif /*INCLUDED_CHECKPOINT_TRAITS_SERIALIZABLE_TRAITS_H*/
