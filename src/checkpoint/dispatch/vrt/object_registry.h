/*
//@HEADER
// *****************************************************************************
//
//                              object_registry.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_VRT_OBJECT_REGISTRY_H
#define INCLUDED_CHECKPOINT_DISPATCH_VRT_OBJECT_REGISTRY_H

#include "checkpoint/common.h"
#include "checkpoint/dispatch/vrt/registry_common.h"
#include "checkpoint/dispatch/reconstructor.h"

#include <functional>
#include <vector>
#include <tuple>

namespace checkpoint { namespace dispatch { namespace vrt {
namespace objregistry {

template <typename T>
using RegistryType = std::vector<
  std::tuple<
    TypeIdx,
    std::function<std::size_t(void)>, // Get the registered object size
    std::function<void*(void)>,       // Do standard allocation for object
    std::function<T*(void*)>          // Construct object on memory
  >
>;

template <typename ObjT>
struct Registrar {
  Registrar();
  TypeIdx index;
};

template <typename ObjT>
struct Type {
  static TypeIdx const idx;
};

template <typename T>
inline RegistryType<T>& getRegistry() {
  static RegistryType<T> reg;
  return reg;
}

template <typename ObjT>
Registrar<ObjT>::Registrar() {
  using BaseType = typename ObjT::_CheckpointVirtualSerializerBaseType;

  auto& reg = getRegistry<BaseType>();
  index = static_cast<TypeIdx>(reg.size());

  debug_checkpoint("object registrar: %zu, %s\n", reg.size(), typeid(ObjT).name());

  reg.emplace_back(
    std::make_tuple(
      index,
      []()          -> std::size_t { return sizeof(ObjT); },
      []()          -> void*       { return std::allocator<ObjT>{}.allocate(1); },
      [](void* buf) -> BaseType*   { return dispatch::Reconstructor<ObjT>::constructAllowFail(buf); }
    )
  );
}

template <typename ObjT>
TypeIdx const Type<ObjT>::idx = Registrar<ObjT>().index;

template <typename T>
inline auto getObjIdx(TypeIdx han) {
  debug_checkpoint("getObjIdx: han=%d, size=%ld\n", han, getRegistry<T>().size());
  return std::get<0>(getRegistry<T>().at(han));
}

template <typename T>
inline auto getObjSize(TypeIdx han) {
  return std::get<1>(getRegistry<T>().at(han));
}

template <typename T>
inline auto getObjAllocate(TypeIdx han) {
  return std::get<2>(getRegistry<T>().at(han));
}

template <typename T>
inline auto getObjConstruct(TypeIdx han) {
  return std::get<3>(getRegistry<T>().at(han));
}

template <typename ObjT>
inline TypeIdx makeObjIdx() {
  return Type<ObjT>::idx;
}

}}}} /* end namespace checkpoint::dispatch::vrt::objregistry */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_VRT_OBJECT_REGISTRY_H*/
