/*
//@HEADER
// *****************************************************************************
//
//                             checkpoint_traits.h
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

#if !defined INCLUDED_CHECKPOINT_TRAITS_CHECKPOINT_TRAITS_H
#define INCLUDED_CHECKPOINT_TRAITS_CHECKPOINT_TRAITS_H

#include <cstdint>

#include "checkpoint/common.h"

#if HAS_DETECTION_COMPONENT
#include "detector_headers.h"
#endif  /*HAS_DETECTION_COMPONENT*/

#if HAS_DETECTION_COMPONENT

namespace checkpoint {

template <typename T>
struct SerializerTraits {
  template <typename U>
  using contiguousBytes_t = decltype(std::declval<U>().contiguousBytes(
    std::declval<void*>(), std::declval<SizeType>(), std::declval<SizeType>()
  ));
  using has_contiguousBytes = detection::is_detected<contiguousBytes_t, T>;

  template <typename U>
  using isSizing_t = decltype(std::declval<U>().isSizing());
  using has_isSizing = detection::is_detected_convertible<bool, isSizing_t, T>;

  template <typename U>
  using isPacking_t = decltype(std::declval<U>().isPacking());
  using has_isPacking = detection::is_detected_convertible<bool, isPacking_t, T>;

  template <typename U>
  using isUnpacking_t = decltype(std::declval<U>().isUnpacking());
  using has_isUnpacking = detection::is_detected_convertible<bool, isUnpacking_t, T>;

  template <typename U>
  using isVirtualDisabled_t = decltype(std::declval<const U>().isVirtualDisabled());
  using has_isVirtualDisabled = detection::is_detected_convertible<bool, isVirtualDisabled_t, T>;

  template <typename U>
  using setVirtualDisabled_t = decltype(std::declval<U>().setVirtualDisabled(std::declval<bool>()));
  using has_setVirtualDisabled = detection::is_detected_convertible<bool, setVirtualDisabled_t, T>;

  // This defines what it means to be a valid serializer
  static constexpr auto const is_valid_serializer =
    has_contiguousBytes::value and
    has_isSizing::value and
    has_isPacking::value and
    has_isUnpacking::value and
    has_isVirtualDisabled::value and
    has_setVirtualDisabled::value;
};

}  // end namespace checkpoint

#endif  /*HAS_DETECTION_COMPONENT*/

#endif /*INCLUDED_CHECKPOINT_TRAITS_CHECKPOINT_TRAITS_H*/
