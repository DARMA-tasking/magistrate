/*
//@HEADER
// *****************************************************************************
//
//                          virtual_serialize_traits.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_VRT_VIRTUAL_SERIALIZE_TRAITS_H
#define INCLUDED_CHECKPOINT_DISPATCH_VRT_VIRTUAL_SERIALIZE_TRAITS_H

#include "checkpoint/common.h"
#include "checkpoint/dispatch/vrt/registry_common.h"

#include <type_traits>

#if HAS_DETECTION_COMPONENT
#include "detector_headers.h"
#endif /*HAS_DETECTION_COMPONENT*/

#if HAS_DETECTION_COMPONENT

namespace checkpoint { namespace dispatch { namespace vrt {

template <typename T>
struct VirtualSerializeTraits {

  template <typename U>
  using has_do_serialize_t = decltype(
    std::declval<U>().doSerialize(
      std::declval<void*>(),
      std::declval<TypeIdx>(),
      std::declval<TypeIdx>()
    )
  );

  using has_do_serialize = detection::is_detected<has_do_serialize_t, T>;

  template <typename U>
  using has_get_index_t = decltype(
    std::declval<U>().getIndex()
  );

  using has_get_index = detection::is_detected<has_get_index_t, T>;

  // This defines what it means to be virtually serializable
  static constexpr auto const is_virtual_serializable =
    has_do_serialize and has_get_index;

  // This defines what it means not to be virtually serializable
  static constexpr auto const is_not_virtual_serializable =
    not is_virtual_serializable;
};

}}} /* end namespace checkpoint::dispatch::vrt */

#else

template <typename T>
struct VirtualSerializeTraits {
  template <
    typename C,
    typename = decltype(
      std::declval<U>().doSerialize(
        std::declval<void*>(),
        std::declval<TypeIdx>(),
        std::declval<TypeIdx>()
      )
    )
  >
  static std::true_type test(int);

  template <typename C>
  static std::false_type test(...);

  static constexpr bool is_virtual_serializable = decltype(test<T>(0))::value;
};

#endif

#endif /*INCLUDED_CHECKPOINT_DISPATCH_VRT_VIRTUAL_SERIALIZE_TRAITS_H*/
