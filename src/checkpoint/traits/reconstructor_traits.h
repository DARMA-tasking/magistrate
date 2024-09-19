/*
//@HEADER
// *****************************************************************************
//
//                            reconstructor_traits.h
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

#if !defined INCLUDED_SRC_CHECKPOINT_TRAITS_RECONSTRUCTOR_TRAITS_H
#define INCLUDED_SRC_CHECKPOINT_TRAITS_RECONSTRUCTOR_TRAITS_H

#include "checkpoint/traits/serializable_traits.h"

#include <type_traits>

namespace checkpoint {

template <typename T>
using isDefaultConsType =
  std::enable_if_t<std::is_default_constructible<T>::value>;

template <typename T>
using isNotDefaultConsType =
  std::enable_if_t<not std::is_default_constructible<T>::value>;

template <typename T>
using isReconstructibleType =
  std::enable_if_t<SerializableTraits<T, void>::is_intrusive_reconstructible>;

template <typename T>
using isNonIntReconstructibleType = std::enable_if_t<
  SerializableTraits<T, void>::is_nonintrusive_reconstructible>;

template <typename T>
using isNotReconstructibleType =
  std::enable_if_t<not SerializableTraits<T, void>::is_reconstructible>;

template <typename T>
using isSpecializedReconstructibleType =
  std::enable_if_t<SerializableTraits<T, void>::is_specialized_reconstructible>;

template <typename T>
using isTaggedConstructibleType =
  std::enable_if_t<SerializableTraits<T, void>::is_tagged_constructible>;

template <typename T>
using isNotTaggedConstructibleType =
  std::enable_if_t<not SerializableTraits<T, void>::is_tagged_constructible>;

template <typename T>
using isConstructible =
  std::enable_if_t<SerializableTraits<T, void>::is_constructible>;

template <typename T>
using isNotConstructible =
  std::enable_if_t<not SerializableTraits<T, void>::is_constructible>;

template <typename T>
using isNotCopyConstructible =
  std::enable_if_t<not std::is_copy_constructible<T>::value>;

template <typename T>
using isCopyConstructible =
  std::enable_if_t<std::is_copy_constructible<T>::value>;

} // namespace checkpoint

#endif /*INCLUDED_SRC_CHECKPOINT_TRAITS_RECONSTRUCTOR_TRAITS_H*/
