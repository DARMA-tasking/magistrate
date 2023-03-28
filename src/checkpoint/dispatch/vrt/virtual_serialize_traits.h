/*
//@HEADER
// *****************************************************************************
//
//                          virtual_serialize_traits.h
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
#include "checkpoint/detector.h"

#include <type_traits>

namespace checkpoint { namespace dispatch { namespace vrt {

template <typename T>
struct VirtualSerializeTraits {

  /**
   * Expect a typedef:
   *  using _CheckpointVirtualSerializerBaseType = T;
   */
  template <typename U>
  using has_base_typedef_t = ::checkpoint::dispatch::vrt::checkpoint_base_type_t<U>;

  using has_base_typedef = detection::is_detected<has_base_typedef_t, T>;

  /**
   * Expect dynamic serialize method:
   *  virtual void _checkpointDynamicSerialize(void*, TypeIdx, TypeIdx);
   */
  template <typename U>
  using has_dynamic_serialize_t = decltype(
    std::declval<U>()._checkpointDynamicSerialize(
      std::declval<void*>(),
      std::declval<TypeIdx>(),
      std::declval<TypeIdx>()
    )
  );

  using has_dynamic_serialize = detection::is_detected<has_dynamic_serialize_t, T>;

  /**
   * Expect dynamic type idx method:
   *  virtual TypeIdx _checkpointDynamicTypeIndex();
   */
  template <typename U>
  using has_dynamic_type_index_t = decltype(
    std::declval<U>()._checkpointDynamicTypeIndex()
  );

  using has_dynamic_type_index =
    detection::is_detected_exact<TypeIdx, has_dynamic_type_index_t, T>;

  // This defines what it means to be virtually serializable
  static constexpr auto const has_virtual_serialize =
    has_dynamic_serialize::value and
    has_dynamic_type_index::value and
    has_base_typedef::value;

  // This defines what it means not to be virtually serializable
  static constexpr auto const has_not_virtual_serialize =
    not has_virtual_serialize;
};

}}} /* end namespace checkpoint::dispatch::vrt */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_VRT_VIRTUAL_SERIALIZE_TRAITS_H*/
