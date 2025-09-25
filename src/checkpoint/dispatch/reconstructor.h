/*
//@HEADER
// *****************************************************************************
//
//                               reconstructor.h
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

#if !defined INCLUDED_SRC_CHECKPOINT_DISPATCH_RECONSTRUCTOR_H
#define INCLUDED_SRC_CHECKPOINT_DISPATCH_RECONSTRUCTOR_H

#include "checkpoint/common.h"
#include "checkpoint/traits/reconstructor_traits.h"
#include "checkpoint/dispatch/reconstructor_tag.h"

#include <tuple>
#include <utility>
#include <cstdlib>

namespace checkpoint { namespace dispatch {

/**
 * \struct InPlaceWrapper
 *
 * \brief A wrapper class for in-place allocations that may not have a
 * corresponding destructor. If a user has an existing allocation that is
 * deserialized into, it may never be destructed as it gets copied or
 * moved. This class encapsulates that allocation and ensures it is properly
 * destroyed, or it explicitly requires \c transferOwnership to be invoked,
 * indicating that the user is now responsible for its destruction. This
 * typically occurs when the user or system places the type in a managed
 * pointer, such as a \c std::unique_ptr<T> with a standard deleter.
 */
template <typename T>
struct InPlaceWrapper {
  // Constructor: Takes ownership of an already constructed object
  explicit InPlaceWrapper(T* inPlaceObject) : object(inPlaceObject) {}

  // Destructor: Calls the destructor of T
  ~InPlaceWrapper() {
    if (object) {
      object->~T(); // Explicitly call the destructor
    }
  }

  // Delete copy constructor and copy assignment to avoid accidental copying
  InPlaceWrapper(const InPlaceWrapper&) = delete;
  InPlaceWrapper& operator=(const InPlaceWrapper&) = delete;

  // Allow move semantics
  InPlaceWrapper(InPlaceWrapper&& other) noexcept : object(nullptr) {
    std::swap(object, other.object);
  }

  InPlaceWrapper& operator=(InPlaceWrapper&& other) noexcept {
    if (this != &other) {
      if (object) {
        object->~T();
      }
      object = nullptr;
      std::swap(object, other.object);
    }
    return *this;
  }

  // Access the wrapped object
  T* operator->() {
    return object;
  }

  const T* operator->() const {
    return object;
  }

  T& operator*() {
    return *object;
  }

  const T& operator*() const {
    return *object;
  }

  /**
   * \brief Tranfer the ownership out of this container. Now the responsbility
   * of where it was transfered to destroy it.
   */
  T* transferOwnership() {
    auto ptr = object;
    object = nullptr;
    return ptr;
  }

private:
  T* object; ///> Pointer to the already constructed object
};

template <typename T>
struct Reconstructor {
  // Default-construct as lowest priority in reconstruction preference
  template <typename U = T>
  static T* constructDefault(void* buf, isDefaultConsType<U>* = nullptr) {
    debug_checkpoint(
      "DeserializerDispatch: default constructor: buf=%p\n", buf
    );
    T* t_ptr = new (buf) T{};
    return t_ptr;
  }

  // Fail, no valid option to constructing T
  template <typename U = T>
  static T* constructDefault(void*, isNotDefaultConsType<U>* = nullptr) {
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
  static T* constructReconstruct(void* buf, isReconstructibleType<U>* = nullptr) {
    debug_checkpoint("DeserializerDispatch: T::reconstruct(): buf=%p\n", buf);
    auto& t = T::reconstruct(buf);
    return &t;
  }

  template <typename U = T>
  static T* constructReconstruct(void* buf, isSpecializedReconstructibleType<U>* = nullptr) {
    debug_checkpoint(
      "DeserializerDispatch: CheckpointReconstructor<T>::reconstruct(): buf=%p\n",
      buf
    );
    T* t = nullptr;
    checkpoint::CheckpointReconstructor<T>::reconstruct(t, buf);
    return t;
  }

  // Non-intrusive reconstruct
  template <typename U = T>
  static T* constructReconstruct(void* buf, isNonIntReconstructibleType<U>* = nullptr) {
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
  static InPlaceWrapper<T> construct(void* buf) {
    return InPlaceWrapper<T>{constructTag<U>(buf)};
  }

  /// Overloads that allow failure to reconstruct so SFINAE overloads don't
  /// static assert out
  template <typename U = T>
  static InPlaceWrapper<T> constructAllowFailImpl(void* buf, isConstructible<U>* = nullptr) {
    return construct<U>(buf);
  }

  template <typename U = T>
  static InPlaceWrapper<T> constructAllowFailImpl(void*, isNotConstructible<U>* = nullptr) {
    constexpr int max_buffer_length = 32768;
    std::unique_ptr<char[]> msg = std::make_unique<char[]>(max_buffer_length);
    snprintf(
      &msg[0],
      max_buffer_length,
      "Checkpoint is failing to reconstruct a class %s, due to it being "
      "abstract or the absence of a suitable constructor (default or tagged) "
      "or reconstruct()",
      typeid(T).name()
    );
    checkpointAssert(false, msg.get());
    return InPlaceWrapper<T>{nullptr};
  }

  // Used for instantiating reconstructor on abstract types that might not be
  // reconstructible
  template <typename U = T>
  static auto constructAllowFail(void* buf) {
    return constructAllowFailImpl<U>(buf);
  }
};

}} /* end namespace checkpoint::dispatch */

#endif /*INCLUDED_SRC_CHECKPOINT_DISPATCH_RECONSTRUCTOR_H*/
