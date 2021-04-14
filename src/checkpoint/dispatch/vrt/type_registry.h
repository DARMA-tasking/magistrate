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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_VRT_TYPE_REGISTRY_H
#define INCLUDED_CHECKPOINT_DISPATCH_VRT_TYPE_REGISTRY_H

#include <cstddef>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace checkpoint { namespace dispatch { namespace vrt { namespace typeregistry {

using TypeNames = std::unordered_map<std::size_t, std::string>;

inline TypeNames& getRegisteredNames() {
  static TypeNames registered_names;
  return registered_names;
}

template <typename ObjT>
struct Registrar {
  Registrar() {
    auto const ti = std::type_index(typeid(ObjT));
    index = ti.hash_code();
    getRegisteredNames()[index] = ti.name();
  }

  std::size_t index;
};

template <typename ObjT>
struct Type {
  static std::size_t const idx;
};

template <typename ObjT>
std::size_t const Type<ObjT>::idx = Registrar<ObjT>().index;

template <typename ObjT>
inline std::size_t getTypeIdx() {
  return Type<ObjT>::idx;
}

inline std::string const& getTypeNameForIdx(const std::size_t typeIdx) {
  return getRegisteredNames()[typeIdx];
}

template <typename ObjT>
inline std::string const& getTypeName() {
  return getTypeNameForIdx(getTypeIdx<ObjT>());
}

}}}} /* end namespace checkpoint::dispatch::vrt::typeregistry */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_VRT_TYPE_REGISTRY_H*/
