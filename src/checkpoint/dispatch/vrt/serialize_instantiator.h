/*
//@HEADER
// *****************************************************************************
//
//                           serialize_instantiator.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_VRT_SERIALIZE_INSTANTIATOR_H
#define INCLUDED_CHECKPOINT_DISPATCH_VRT_SERIALIZE_INSTANTIATOR_H

#include "checkpoint/dispatch/vrt/serializer_registry.h"
#include "checkpoint/dispatch/vrt/link_derived_to_base_idx.h"

namespace checkpoint { namespace dispatch { namespace vrt {

template <typename ObjT, typename SerializerT>
static inline void instantiateObjSerializerImpl() {
  dispatch::vrt::serializer_registry::makeObjIdx<ObjT, SerializerT>();

  // If the \c SerializerT static registrations order happen differently for a
  // given \c BaseT and \c DerviedT , you will get the wrong serializer when
  // using the base_idx in the derived class. Thus, link the correct base
  // serialize type idx with a given derived type idx during
  // sizing/packing. Then, when it unpacks, it can find the right one when
  // running on the derived type using the base idx.
  using BaseType = typename ObjT::_CheckpointVirtualSerializerBaseType;
  linkDerivedToBase<SerializerT, ObjT, BaseType>();
}

template <typename ObjT, typename... Ts>
struct InstantiateTupleHelper;

template <typename ObjT, typename T, typename... Ts>
struct InstantiateTupleHelper<ObjT, T, Ts...> {
  static void _recur_register() {
    instantiateObjSerializerImpl<ObjT, T>();
    InstantiateTupleHelper<ObjT, Ts...>::_recur_register();
  }
};

template <typename ObjT>
struct InstantiateTupleHelper<ObjT> {
  static void _recur_register() {
    // recursion done.
  }
};

}}} /* end namespace checkpoint::dispatch::vrt */

namespace checkpoint {

template <typename ObjT, typename... SerializerTs>
inline void instantiateObjSerializer() {
  dispatch::vrt::InstantiateTupleHelper<ObjT, SerializerTs...>::_recur_register();
}

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_VRT_SERIALIZE_INSTANTIATOR_H*/
