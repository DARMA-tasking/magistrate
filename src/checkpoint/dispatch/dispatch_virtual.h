/*
//@HEADER
// *****************************************************************************
//
//                              dispatch_virtual.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_VIRTUAL_H
#define INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_VIRTUAL_H

#include "checkpoint/dispatch/dispatch.h"
#include "checkpoint/dispatch/vrt/base.h"
#include "checkpoint/dispatch/vrt/derived.h"

#include <vector>
#include <tuple>
#include <functional>

namespace checkpoint {

template <typename ObjT, typename SerializerT>
void instantiate() {
  dispatch::vrt::serializer_registry::makeObjIdx<ObjT, SerializerT>();
}

/**
 * \brief A function to handle serialization of objects of a mix of
 * types in a virtual inheritance hierarchy
 *
 * This will automatically record the exact derived type at
 * serialization, and reconstruct objects accordingly at
 * deserialization. The constructor will be passed an argument of
 * type SERIALIZE_CONSTRUCT_TAG.
 */
template <typename BaseT, typename SerializerT>
void virtualSerialize(BaseT*& base, SerializerT& s) {
  using namespace dispatch::vrt;

  TypeIdx entry = -1;
  if (not s.isUnpacking()) {
    entry = base->_checkpointDynamicTypeIndex();
  }

  s | entry;

  debug_checkpoint("virtualSerialize: entry=%d\n", entry);

  if (s.isUnpacking()) {
    auto lam = objregistry::getObjIdx<BaseT>(entry);
    auto ptr = std::get<1>(lam)();
    base = ptr;
  }

  base->_checkpointDynamicSerialize(&s, dispatch::vrt::serializer_registry::makeObjIdx<BaseT, SerializerT>(), -1);
}

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_DISPATCH_VIRTUAL_H*/
