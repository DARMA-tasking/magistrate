/*
//@HEADER
// *****************************************************************************
//
//                               reconstructor.h
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
#include "checkpoint/traits/reconstructor_traits.h"
#include "checkpoint/dispatch/reconstructor_tag.h"

#include <tuple>
#include <cstdlib>

namespace checkpoint { namespace dispatch {

template <typename T>
struct Reconstructor {
  // Default-construct as lowest priority in reconstruction preference
  template <typename U = T>
  static T* constructDefault(
    void* buf,
    typename ReconstructorTraits<T>::template isDefaultConsType<U>* = nullptr
  ) {
    debug_checkpoint(
      "DeserializerDispatch: default constructor: buf=%p\n", buf
    );
    T* t_ptr = new (buf) T{};
    return t_ptr;
  }

  // Fail, no valid option to constructing T
  template <typename U = T>
  static T* constructDefault(
    void* buf,
    typename ReconstructorTraits<T>::template isNotDefaultConsType<U>* = nullptr
  ) {
    static_assert(
      SerializableTraits<U, void>::is_tagged_constructible or
        SerializableTraits<U, void>::is_reconstructible or
        std::is_default_constructible<U>::value,
      "Either a default constructor, reconstruct() function, or tagged "
      "constructor are required for de-serialization"
    );
    return nullptr;
  }

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
  static T* constructReconstruct(
    void* buf,
    typename ReconstructorTraits<T>::template isReconstructibleType<U>* = nullptr
  ) {
    debug_checkpoint("DeserializerDispatch: T::reconstruct(): buf=%p\n", buf);
    auto& t = T::reconstruct(buf);
    return &t;
  }

  // Non-intrusive reconstruct
  template <typename U = T>
  static T* constructReconstruct(
    void* buf,
    typename ReconstructorTraits<T>::template isNonIntReconstructibleType<U>* = nullptr
  ) {
    debug_checkpoint(
      "DeserializerDispatch: non-int reconstruct(): buf=%p\n", buf
    );
    T* t = nullptr;
    // Explicitly call bare to invoke ADL
    reconstruct(t, buf);
    return t;
  }

  /// Non-reconstruct pass-through
  template <typename U = T>
  static T* constructReconstruct(
    void* buf,
    typename ReconstructorTraits<T>::template isNotReconstructibleType<U>* = nullptr
  ) {
    return constructDefault<U>(buf);
  }

  /// Tagged constructor
  template <typename U = T>
  static T* constructTag(
    void* buf,
    typename ReconstructorTraits<T>::template isTaggedConstructibleType<U>* = nullptr
  ) {
    debug_checkpoint("DeserializerDispatch: tagged constructor: buf=%p\n", buf);
    T* t_ptr = new (buf) T{SERIALIZE_CONSTRUCT_TAG{}};
    return t_ptr;
  }

  /// Non-tagged constructor pass-through
  template <typename U = T>
  static T* constructTag(
    void* buf,
    typename ReconstructorTraits<T>::template isNotTaggedConstructibleType<U>* = nullptr
  ) {
    return constructReconstruct<U>(buf);
  }

  template <typename U = T>
  static T* construct(void* buf) {
    return constructTag<U>(buf);
  }

  /// Overloads that allow failure to reconstruct so SFINAE overloads don't
  /// static assert out
  template <typename U = T>
  static T* constructAllowFailImpl(
    void* buf,
    typename ReconstructorTraits<T>::template isConstructible<U>* = nullptr
  ) {
    return construct<U>(buf);
  }

  template <typename U = T>
  static T* constructAllowFailImpl(
    void* buf,
    typename ReconstructorTraits<T>::template isNotConstructible<U>* = nullptr
  ) {
    std::unique_ptr<char[]> msg = std::make_unique<char[]>(32768);
    sprintf(
      &msg[0],
      "Checkpoint is failing to reconstruct a class %s, due to it being "
      "abstract or the absence of a suitable constructor (default or tagged) "
      "or reconstruct()",
      typeid(T).name()
    );
    checkpointAssert(false, msg.get());
    return nullptr;
  }

  // Used for instantiating reconstructor on abstract types that might not be
  // reconstructible
  template <typename U = T>
  static T* constructAllowFail(void* buf) {
    return constructAllowFailImpl<U>(buf);
  }
};

}} /* end namespace checkpoint::dispatch */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_RECONSTRUCTOR_H*/
