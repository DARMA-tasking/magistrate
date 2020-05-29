/*
//@HEADER
// *****************************************************************************
//
//                            serializer_registry.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_VRT_SERIALIZER_REGISTRY_H
#define INCLUDED_CHECKPOINT_DISPATCH_VRT_SERIALIZER_REGISTRY_H

#include "checkpoint/common.h"
#include "checkpoint/dispatch/vrt/registry_common.h"

#include <functional>
#include <vector>
#include <tuple>

namespace checkpoint { namespace dispatch { namespace vrt {
namespace serializer_registry {

template <typename T>
struct SerializerEntry {

  template <typename Serializer>
  SerializerEntry(
    TypeIdx in_this_idx,
    TypeIdx in_base_idx,
    Serializer&& in_serializer
  ) : this_idx_(in_this_idx),
      base_idx_(in_base_idx),
      serializer_(in_serializer)
  { }

  TypeIdx this_idx_ = no_type_idx;            /**< This entry type idx */
  TypeIdx base_idx_ = no_type_idx;            /**< The base class type idx */
  std::function<void(void*, T&)> serializer_; /**< Type-erased serialize dispatch */
};

template <typename ObjT>
using RegistryType = std::vector<SerializerEntry<ObjT>>;

template <typename ObjT, typename SerializerT>
struct Registrar {
  Registrar();
  TypeIdx index;
};

template <typename ObjT, typename SerializerT>
struct Type {
  static TypeIdx const idx;
};

template <typename ObjT>
inline RegistryType<ObjT>& getRegistry() {
  static RegistryType<ObjT> reg;
  return reg;
}

template <typename ObjT, typename SerializerT>
inline TypeIdx makeObjIdx() {
  return Type<ObjT, SerializerT>::idx;
}

template <typename ObjT, typename SerializerT>
Registrar<ObjT, SerializerT>::Registrar() {
  auto& reg = getRegistry<ObjT>();
  index = static_cast<TypeIdx>(reg.size());

  debug_checkpoint(
    "serializer registrar: %zu, ObjT=%s SerializerT=%s\n",
    reg.size(),
    typeid(ObjT).name(),
    typeid(SerializerT).name()
  );

  reg.emplace_back(
    SerializerEntry<ObjT>{
      index,
      no_type_idx, // Set later when linkDerivedToBase is called
      [=](void* s, ObjT& obj) {
        auto& ser = *reinterpret_cast<SerializerT*>(s);
        // Disable virtual serializer dispatch because we are already in a
        // virtualSerialize and otherwise we will recurse indefinitely
        ser.setVirtualDisabled(true);
        ser | obj;
      }
    }
  );
}

template <typename ObjT, typename SerializerT>
TypeIdx const Type<ObjT, SerializerT>::idx =
  Registrar<ObjT, SerializerT>().index;

template <typename ObjT>
inline auto& getObjIdxRef(TypeIdx han) {
  checkpointAssert(
    han < static_cast<TypeIdx>(getRegistry<ObjT>().size()),
    "Missing type idx in registry. A derived type was not registered with "
    "checkpoint for a given serializer"
  );
  return getRegistry<ObjT>().at(han);
}

template <typename ObjT>
inline auto getObjIdx(TypeIdx han) {
  auto elm = getObjIdxRef<ObjT>(han);
  return elm.serializer_;
}

template <typename DerivedT>
inline std::function<void(void*, DerivedT&)> getBaseIdx(TypeIdx base_idx) {
  auto& reg = getRegistry<DerivedT>();
  for (auto entry : reg) {
    if (entry.base_idx_ == base_idx) {
      return entry.serializer_;
    }
  }
  checkpointAssert(
    false, "Error, could not find corresponding entry in derived for base"
  );
  return nullptr;
}

}}}} /* end namespace checkpoint::dispatch::vrt::serializer_registry */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_VRT_SERIALIZER_REGISTRY_H*/
