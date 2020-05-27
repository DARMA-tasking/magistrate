/*
//@HEADER
// *****************************************************************************
//
//                               reconstructor.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_RECONSTRUCTOR_H
#define INCLUDED_CHECKPOINT_DISPATCH_RECONSTRUCTOR_H

#include "checkpoint/common.h"
#include "checkpoint/traits/serializable_traits.h"
#include "checkpoint/dispatch/reconstructor_tag.h"

#include <type_traits>
#include <tuple>
#include <cstdlib>

namespace checkpoint { namespace dispatch {

template <typename T>
struct Reconstructor {
  template <typename U>
  using isDefaultConsType =
  typename std::enable_if<std::is_default_constructible<U>::value, T>::type;

  template <typename U>
  using isNotDefaultConsType =
  typename std::enable_if<not std::is_default_constructible<U>::value, T>::type;

  // If we have the detection component, we can more precisely check for
  // reconstuctibility
  #if HAS_DETECTION_COMPONENT
    template <typename U>
    using isReconstructibleType =
    typename std::enable_if<SerializableTraits<U>::is_reconstructible, T>::type;

    template <typename U>
    using isNonIntReconstructibleType =
    typename std::enable_if<
      SerializableTraits<U>::is_nonintrusive_reconstructible, T
    >::type;

    template <typename U>
    using isNotReconstructibleType =
    typename std::enable_if<
      not SerializableTraits<U>::is_nonintrusive_reconstructible and
      not SerializableTraits<U>::is_reconstructible,
      T
    >::type;

    template <typename U>
    using isTaggedConstructibleType =
    typename std::enable_if<SerializableTraits<U>::is_tagged_constructible, T>::type;

    template <typename U>
    using isNotTaggedConstructibleType =
    typename std::enable_if<not SerializableTraits<U>::is_tagged_constructible, T>::type;
  #endif

  // Default-construct as lowest priority in reconstruction preference
  template <typename U = T>
  static T* constructDefault(void* buf, isDefaultConsType<U>* = nullptr) {
    debug_checkpoint("DeserializerDispatch: default constructor: buf=%p\n", buf);
    T* t_ptr = new (buf) T{};
    return t_ptr;
  }

  // Fail, no valid option to constructing T
  template <typename U = T>
  static T* constructDefault(void* buf, isNotDefaultConsType<U>* = nullptr) {
    #if HAS_DETECTION_COMPONENT
    static_assert(
      SerializableTraits<U>::is_tagged_constructible or
      SerializableTraits<U>::is_reconstructible or
      SerializableTraits<U>::is_nonintrusive_reconstructible or
      std::is_default_constructible<U>::value,
      "Either a default constructor, reconstruct() function, or tagged "
      "constructor are required for de-serialization"
    );
    #else
    static_assert(
      std::is_default_constructible<U>::value,
      "A default constructor is required for de-serialization. To enable "
      "reconstruct or tagged constructors, you must compile with the "
      "detection component."
    );
    #endif
    return nullptr;
  }

  #if HAS_DETECTION_COMPONENT

  /*
   * Try to reconstruct with the following precedence:
   *
   *   - Tagged constructor: T(SERIALIZE_CONSTRUCT_TAG{})
   *   - Reconstruct: T::reconstruct(buf) or reconstruct(t, buf)
   *   - Default constructor: T()
   *
   * If none of these options work, static assert failure
   */

  // Intrusive reconstruct
  template <typename U = T>
  static T* constructReconstruct(void* buf, isReconstructibleType<U>* = nullptr) {
    debug_checkpoint("DeserializerDispatch: T::reconstruct(): buf=%p\n", buf);
    auto& t = T::reconstruct(buf);
    return &t;
  }

  // Non-intrusive reconstruct
  template <typename U = T>
  static T* constructReconstruct(void* buf, isNonIntReconstructibleType<U>* = nullptr) {
    debug_checkpoint("DeserializerDispatch: non-int reconstruct(): buf=%p\n", buf);
    T* t = nullptr;
    // Explicitly call bare to invoke ADL
    reconstruct(t,buf);
    return t;
  }

  /// Non-reconstruct pass-through
  template <typename U = T>
  static T* constructReconstruct(void* buf, isNotReconstructibleType<U>* = nullptr) {
    return constructDefault<U>(buf);
  }

  /// Tagged constructor
  template <typename U = T>
  static T* constructTag(void* buf, isTaggedConstructibleType<U>* = nullptr) {
    debug_checkpoint("DeserializerDispatch: tagged constructor: buf=%p\n", buf);
    T* t_ptr = new (buf) T{SERIALIZE_CONSTRUCT_TAG{}};
    return t_ptr;
  }

  /// Non-tagged constructor pass-through
  template <typename U = T>
  static T* constructTag(void* buf, isNotTaggedConstructibleType<U>* = nullptr) {
    return constructReconstruct<U>(buf);
  }

  template <typename U = T>
  static T* construct(void* buf) {
    return constructTag<U>(buf);
  }

  #else

  template <typename U = T>
  static T* construct(void* buf) {
    return constructDefault<U>(buf);
  }

  #endif


};

}} /* end namespace checkpoint::dispatch */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_RECONSTRUCTOR_H*/
