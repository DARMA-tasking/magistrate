/*
//@HEADER
// *****************************************************************************
//
//                                 sanitizer.h
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

#if !defined INCLUDED_CHECKPOINT_SERIALIZERS_SANITIZER_H
#define INCLUDED_CHECKPOINT_SERIALIZERS_SANITIZER_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/base_serializer.h"

#include <memory>

namespace checkpoint { namespace sanitizer {

/**
 * \struct Runtime
 *
 * \brief Base class for sanitizer runtime implemented by the sanitizer
 */
struct Runtime {

  /**
   * \brief Check that a member is serialized
   *
   * \param[in] addr the memory address of the element
   * \param[in] name the name of the element
   * \param[in] tinfo the typeinfo of the element
   */
  virtual void checkMember(void* addr, std::string name, std::string tinfo) {}

  /**
   * \brief Inform sanitizer that a member's serialization is skipped
   *
   * \param[in] addr the memory address of the element
   * \param[in] name the name of the element
   * \param[in] tinfo the typeinfo of the element
   */
  virtual void skipMember(void* addr, std::string name, std::string tinfo) {}

  /**
   * \brief Inform sanitizer that a member is serialized
   *
   * \param[in] addr the memory address of the element
   * \param[in] num the number of elements
   * \param[in] tinfo the typeinfo of the element
   */
  virtual void isSerialized(void* addr, std::size_t num, std::string tinfo) {}

  /**
   * \brief Push a stack frame of the current serializer context we are entering
   *
   * \param[in] tinfo the name of the type recursed into
   */
  virtual void push(std::string tinfo) {}

  /**
   * \brief Pop a stack frame of the current serializer context we are leaving
   *
   * \param[in] tinfo the name of the type recursed out of
   */
  virtual void pop(std::string tinfo) {}

};

/// pimpl to runtime that contains runtime sanitizer logic
std::unique_ptr<Runtime> rt_ = nullptr;

}} /* end namespace checkpoint::sanitizer*/

namespace checkpoint { namespace serializers {

/**
 * \struct SanitizerDispatch
 *
 * \brief Custom dispatcher at compile-time for sanitization pass injected into
 * user code
 */
template <typename SerializerT, typename T>
struct SanitizerDispatch {

  /**
   * \brief Overload for traversing an intrusive serialize function of \c t
   *
   * \param[in] s the serializer
   * \param[in] t the element
   */
  static void serializeIntrusive(SerializerT& s, T& t);

  /**
   * \brief Overload for traversing a non-intrusive serialize function over an
   * enum \c t
   *
   * \param[in] s the serializer
   * \param[in] t the element
   */
  static void serializeNonIntrusiveEnum(SerializerT& s, T& t);

  /**
   * \brief Overload for traversing a non-intrusive serialize function of \c t
   *
   * \param[in] s the serializer
   * \param[in] t the element
   */
  static void serializeNonIntrusive(SerializerT& s, T& t);
};

/**
 * \struct BaseSanitizer
 *
 * \brief The base class for the sanitizer pass that applies the typed
 * overloaded for the sanitization traversal.
 */
struct BaseSanitizer : checkpoint::Serializer {

  /// The custom dispatcher that intercepts traversal at compile-time
  template <typename U, typename V>
  using DispatcherType = SanitizerDispatch<U, V>;

  /**
   * \internal \brief Construct the default sanitizer
   */
  BaseSanitizer() : BaseSanitizer(checkpoint::eSerializationMode::Sanitizing) {}

  /**
   * \internal \brief Construct a \c BaseSanitizer in a certain serialization
   * mode
   *
   * \param[in] in_mode the serialization mode
   */
  explicit BaseSanitizer(checkpoint::eSerializationMode in_mode)
    : checkpoint::Serializer(in_mode)
  { }

  /**
   * \brief Check that member is actually serialized
   *
   * \param[in] t address of the element
   * \param[in] t_name pretty print of the name for diagnostic output to user
   */
  template <typename T>
  void check(T& t, std::string t_name);

  /**
   * \brief Tell the sanitizer that a certain member will be skipped. Typically,
   * this is called if the data does not need to be serialized or is indirectly
   * saved.
   *
   * \param[in] t address of the element
   * \param[in] t_name pretty print of the name for diagnostic output to user
   */
  template <typename T>
  void skip(T& t, std::string t_name = "");

  /**
   * \brief Traverse contiguous data for sanitization purposes
   *
   * \param[in,out] s serializer
   * \param[in] t pointer to the element start
   * \param[in] num_elsm the number of elements starting at \c t
   */
  template <typename SerializerT, typename T>
  void contiguousTyped(SerializerT& s, T* t, std::size_t num_elms);

  /**
   * \brief Do nothing on contiguous bytes
   */
  void contiguousBytes(void*, std::size_t, std::size_t) { }

  /**
   * \internal \brief Clean \c t and cast to \c void*
   *
   * \param[in] t the element
   *
   * \return untyped pointer to the element
   */
  template <typename T>
  static void* cleanTypeToVoid(T&& t);
};

/**
 * \struct Sanitizer
 *
 * \brief The special type for sanitization that is specialized for traversal
 * over a class. \c BaseSanitizer does all the heavy lifting---this just
 * providing the compiler dispatch to the correct template overload.
 */
struct Sanitizer : BaseSanitizer { };

/**
 * \struct NonSanitizingSanitizer
 *
 * \brief A type for sanitization that explicitly does not match the partial
 * specialization to purposely invoke the regular serialization method on a
 * class.
 */
struct NonSanitizingSanitizer : BaseSanitizer { };

}} /* end namespace checkpoint::serializers */

#include "checkpoint/serializers/sanitizer.impl.h"

#endif /*INCLUDED_CHECKPOINT_SERIALIZERS_SANITIZER_H*/
