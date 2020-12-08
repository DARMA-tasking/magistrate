/*
//@HEADER
// *****************************************************************************
//
//                               sanitizer.impl.h
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

#if !defined INCLUDED_CHECKPOINT_SERIALIZERS_SANITIZER_IMPL_H
#define INCLUDED_CHECKPOINT_SERIALIZERS_SANITIZER_IMPL_H

#include "checkpoint/serializers/sanitizer.h"
#include "checkpoint/dispatch/clean_type.h"

#include <typeinfo>

namespace checkpoint { namespace serializers {

template <typename T>
void* cleanTypeToVoid(T&& t) {
  return reinterpret_cast<void*>(dispatch::cleanType(&t));
}

template <typename SerT, typename T>
void SanitizerDispatch<SerT,T>::serializeIntrusive(SerT& s, T& t) {
  if (not sanitizer::enabled()) {
    return;
  }

  // Declare this member as serialized
  sanitizer::rt()->isSerialized(cleanTypeToVoid(t), 1, typeid(T).name());

  // Push this type on the stack since we are about to traverse it
  sanitizer::rt()->push(typeid(T).name());

  // Recurse with the current serializer (a sanitizing pass)
  t.serialize(s);

  // A bit of a trick...recurse with a non-sanitizing-specific overload to
  // resolve actual \c serialize methods without invoking the partial
  // specialization
  NonSanitizingSanitizer nss{s};
  t.serialize(nss);

  // Pop off of stack
  sanitizer::rt()->pop(typeid(T).name());
}

template <typename SerT, typename T>
void SanitizerDispatch<SerT,T>::serializeNonIntrusiveEnum(SerT& s, T& t) {
  if (not sanitizer::enabled()) {
    return;
  }

  // Declare this enum as serialized
  sanitizer::rt()->isSerialized(cleanTypeToVoid(t), 1, typeid(T).name());
}

template <typename SerT, typename T>
void SanitizerDispatch<SerT,T>::serializeNonIntrusive(SerT& s, T& t) {
  if (not sanitizer::enabled()) {
    return;
  }

  // Declare this enum as serialized
  sanitizer::rt()->isSerialized(cleanTypeToVoid(t), 1, typeid(T).name());

  // Push this type on the stack since we are about to traverse it
  sanitizer::rt()->push(typeid(T).name());

  // Recurse with the current serializer (a sanitizing pass)
  serialize(s, t);

  // A bit of a trick...recurse with a non-sanitizing-specific overload to
  // resolve actual \c serialize methods without invoking the partial
  // specialization
  NonSanitizingSanitizer nss{s};
  serialize(nss, t);

  // Pop off of stack
  sanitizer::rt()->pop(typeid(T).name());
}

template <typename T>
void BaseSanitizer::check(T& t, std::string t_name) {
  sanitizer::rt()->checkMember(cleanTypeToVoid(t), t_name, typeid(T).name());
}

template <typename T>
void BaseSanitizer::skip(T& t, std::string t_name) {
  sanitizer::rt()->skipMember(cleanTypeToVoid(t), t_name, typeid(T).name());
}

template <typename SerializerT, typename T>
void BaseSanitizer::contiguousTyped(SerializerT&, T* t, std::size_t num_elms) {
  sanitizer::rt()->isSerialized(
    reinterpret_cast<void*>(t), num_elms, typeid(t).name()
  );
}

}} /* end namespace checkpoint::serializers */

#endif /*INCLUDED_CHECKPOINT_SERIALIZERS_SANITIZER_IMPL_H*/
