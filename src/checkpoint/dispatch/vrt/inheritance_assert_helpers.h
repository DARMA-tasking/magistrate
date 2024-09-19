/*
//@HEADER
// *****************************************************************************
//
//                         inheritance_assert_helpers.h
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

#if !defined INCLUDED_SRC_CHECKPOINT_DISPATCH_VRT_INHERITANCE_ASSERT_HELPERS_H
#define INCLUDED_SRC_CHECKPOINT_DISPATCH_VRT_INHERITANCE_ASSERT_HELPERS_H

#include "checkpoint/common.h"

#include <string>

namespace checkpoint { namespace dispatch { namespace vrt {

template <typename ObjT>
inline void assertTypeIdxMatch(TypeIdx const expected_idx) {
#if CHECKPOINT_ASSERT_ENABLED
  auto obj_idx = objregistry::makeObjIdx<ObjT>();

  static std::string debug_str =
    std::string("Type idx for object \"") + typeid(ObjT).name() +
    "\" does not matched expected value. "
    "You are probably missing a SerializableBase<T> or SerializableDerived<T> "
    "in the virtual class hierarchy; or, if you are using macros: "
    "checkpoint_virtual_serialize_root() or checkpoint_virtual_serialize_derived_from(..)";
  checkpointAssert(
    obj_idx == expected_idx or expected_idx == no_type_idx, debug_str.c_str()
  );
#endif
}

}}} /* end namespace checkpoint::dispatch::vrt */

#endif /*INCLUDED_SRC_CHECKPOINT_DISPATCH_VRT_INHERITANCE_ASSERT_HELPERS_H*/
