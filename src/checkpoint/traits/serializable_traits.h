/*
//@HEADER
// *****************************************************************************
//
//                            serializable_traits.h
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
#include "checkpoint/detector.h"

#include <cstdint>
#include <cassert>

/*
 * Start the traits class with all the archetypes that can be detected for
 * serialization
 */

namespace checkpoint {

template <typename T>
void reconstruct(T) { checkpointAssert(false, "This code should be unreachable"); }

template <typename T>
struct CheckpointReconstructor;

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
struct isByteCopyable : detail::isByteCopyableImpl<T>::has_byteCopyTraitTrue {};

template <typename T, typename S = checkpoint::Serializer<>>
struct SerializableTraits {
  /**
   * Start with detection of "serialize" overloads, intrusive and non-intrusive.
   *
   * NOTE: Detection of \c serializeParent and \c serializeThis are no longer
   * used on "develop" in VT (eliminated with message serialization
   * refactoring), but remain on the release branch for 1.0.0. Thus, they must
   * stay for now.
   */

  // Regular serialize detection
  template <typename U, typename V>
  using serialize_t = decltype(
    std::declval<U>().template serialize<V>(std::declval<V&>())
  );
  using has_serialize = detection::is_detected<serialize_t, T, S>;

  template <typename U, typename V>
  using nonintrustive_serialize_t = decltype(serialize<V>(
    std::declval<V&>(),
    std::declval<U&>()
  ));
  using has_nonintrustive_serialize =
    detection::is_detected<nonintrustive_serialize_t, T, S>;

  // serialize{Parent,This} detection
  template <typename U, typename V>
  using serializeParent_t =
    decltype(std::declval<U>().serializeParent(std::declval<V&>()));
  using has_serializeParent = detection::is_detected<serializeParent_t, T, S>;

  template <typename U, typename V>
  using serializeThis_t =
    decltype(std::declval<U>().serializeThis(std::declval<V&>()));
  using has_serializeThis = detection::is_detected<serializeThis_t, T, S>;

  // This defines what it means to have an intrusive serialize
  static constexpr auto const has_serialize_instrusive = has_serialize::value;

  // This defines what it means to have a non-intrusive serialize
  static constexpr auto const has_serialize_noninstrusive =
    has_nonintrustive_serialize::value;

  // This defines what it means to have some type of serialize function
  static constexpr auto const has_serialize_function =
    has_serialize_instrusive or has_serialize_noninstrusive;

  // This defines what it means to have parent serializability
  static constexpr auto const has_parent_serialize = has_serializeParent::value;

  // This defines what it means to have "this" serializability
  static constexpr auto const has_this_serialize = has_serializeThis::value;

  // This defines what it means to have both parent and "this" serializability
  static constexpr auto const has_split_serialize =
    has_parent_serialize and has_this_serialize;

  /**
   * Detect byte-copyability trait and std::is_arithmetic, used for types that
   * do not have a serialize method.
   */
  using has_byteCopyTraitTrue = isByteCopyable<T>;

  template <typename U>
  using has_isArith = std::is_arithmetic<U>;

  // This defines what it means to be serializable without a serialize method
  static constexpr auto const is_bytecopyable =
    has_byteCopyTraitTrue::value or has_isArith<T>::value;

  /**
   * Detect different types of re-constructibility: default constructors,
   * explicitly tagged constructors for serializers, and reconstruct methods.
   */
  template <typename U>
  using constructor_t = decltype(U());
  using has_default_constructor = detection::is_detected<constructor_t, T>;

  template <typename U>
  using tagged_constructor_t = decltype(U(std::declval<SERIALIZE_CONSTRUCT_TAG>()));
  using has_tagged_constructor = detection::is_detected<tagged_constructor_t, T>;

  template <typename U>
  using reconstruct_t = decltype(U::reconstruct(std::declval<void*>()));
  using has_reconstruct = detection::is_detected_exact<T&, reconstruct_t, T>;

  template <typename U>
  using nonintrustive_reconstruct_t = decltype(
    reconstruct(
      std::declval<U*&>(),
      std::declval<void*>()
    )
  );
  using has_nonintrusive_reconstruct =
    detection::is_detected<nonintrustive_reconstruct_t, T>;

  template <typename U>
  using specialized_reconstruct_t = decltype(
    checkpoint::CheckpointReconstructor<U>::reconstruct(
      std::declval<U*&>(),
      std::declval<void*>()
    )
  );
  using has_specialized_reconstruct =
    detection::is_detected<specialized_reconstruct_t, T>;

  // This defines what it means to be default constructible
  static constexpr auto const is_default_constructible =
    has_default_constructor::value;

  // If the class has tagged constructor especially for constructing
  static constexpr auto const is_tagged_constructible =
    has_tagged_constructor::value;

  // This defines what it means to be intrusively reconstructible
  static constexpr auto const is_intrusive_reconstructible = has_reconstruct::value;

  // This defines what it means to be non-intrusively reconstructible
  static constexpr auto const is_nonintrusive_reconstructible =
    has_nonintrusive_reconstruct::value /*and not has_reconstruct::value*/;

  // This defines what it means to be specialized reconstructible
  static constexpr auto const is_specialized_reconstructible =
    has_specialized_reconstruct::value;

  // This defines what it means to be reconstructible in general
  static constexpr auto const is_reconstructible =
    has_nonintrusive_reconstruct::value or
    has_reconstruct::value or
    has_specialized_reconstruct::value;

  // This defines what is means to be generally constructible for
  // de-serialization
  static constexpr auto const is_constructible =
    is_default_constructible or is_reconstructible or is_tagged_constructible;

  /**
   * Finally, combine all these into a single definition of is_serializable
   */
  static constexpr auto const is_serializable =
    has_serialize_function and is_constructible;

  static constexpr auto const is_traversable =
    has_serialize_function or is_bytecopyable;
};

}  // end namespace checkpoint

#endif /*INCLUDED_CHECKPOINT_TRAITS_SERIALIZABLE_TRAITS_H*/
