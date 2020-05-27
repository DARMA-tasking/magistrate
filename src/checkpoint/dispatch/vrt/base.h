/*
//@HEADER
// *****************************************************************************
//
//                                    base.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_VRT_BASE_H
#define INCLUDED_CHECKPOINT_DISPATCH_VRT_BASE_H

#include "checkpoint/dispatch/vrt/registry_common.h"
#include "checkpoint/dispatch/vrt/base_base.h"
#include "checkpoint/dispatch/vrt/static_dispatch_typeidx.h"
#include "checkpoint/dispatch/vrt/object_registry.h"
#include "checkpoint/dispatch/vrt/serializer_registry.h"

namespace checkpoint { namespace dispatch { namespace vrt {

/**
 * \brief A class at the base of an inheritance hierarchy should inherit from this
 *
 * \param BaseT the base class itself, following CRTP, to provide a
 * common identifier of the whole hierarchy
 */
template <typename BaseT>
struct SerializableBase : SerializableBaseBase {
  using SerDerBaseType = BaseT;

  void _checkpointDynamicSerialize(
    void* s, TypeIdx serializer_idx, TypeIdx expected_idx
  ) override {
    auto base_idx = objregistry::makeObjIdx<BaseT>();

    if (base_idx != expected_idx) {
      printf("broken assert %s\n", typeid(BaseT).name());
    }

    assert(base_idx == expected_idx && "Check in base");

    auto dispatcher = serializer_registry::getObjIdx<BaseT>(serializer_idx);
    dispatcher(s, *static_cast<BaseT*>(this));
  }

  TypeIdx _checkpointDynamicTypeIndex() override {
    return DispatchTypeIdx<BaseT>::get();
  }
};

}}} /* end namespace checkpoint::dispatch::vrt */

namespace checkpoint {

template <typename BaseT>
using SerializableBase = dispatch::vrt::SerializableBase<BaseT>;

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_VRT_BASE_H*/
