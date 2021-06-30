/*
//@HEADER
// *****************************************************************************
//
//                              type_registry.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_TYPE_REGISTRY_H
#define INCLUDED_CHECKPOINT_DISPATCH_TYPE_REGISTRY_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

#if defined __GNUG__
  #include <cstdlib>
  #include <cxxabi.h>
  #include <memory>
#endif

namespace checkpoint { namespace dispatch { namespace typeregistry {

#if defined __GNUG__

inline std::string demangle(const char* name) {
  int status;
  std::unique_ptr<char, void (*)(void*)> res{
    abi::__cxa_demangle(name, NULL, NULL, &status), std::free
  };

  constexpr int success = 0;
  return (status == success) ? res.get() : name;
}

#else

inline std::string demangle(const char* name) {
  return name;
}

#endif

using DecodedIndex = std::uint32_t;
using TypeNames = std::unordered_map<DecodedIndex, std::string>;

inline TypeNames& getRegisteredNames() {
  static TypeNames registered_names;
  return registered_names;
}

using TypeIndex = std::uint16_t;

inline TypeIndex getIndex() {
  static TypeIndex index = 0;
  return index++;
}

using DeadType = std::uint32_t;
static constexpr DeadType dead_mask = 0xFF0000FF;
static constexpr DeadType dead_mark = 0xDE0000AD;

inline bool validateIndex(const DecodedIndex index) {
  return dead_mark == (index & dead_mask);
}

template <typename ObjT>
struct Registrar {
  Registrar() {
    index = (getIndex() << 8) | dead_mark;
    getRegisteredNames()[index] = demangle(typeid(ObjT).name());
  }

  DecodedIndex index;
};

template <typename ObjT>
struct Type {
  static DecodedIndex const idx;
};

template <typename ObjT>
DecodedIndex const Type<ObjT>::idx = Registrar<ObjT>().index;

template <typename ObjT>
inline DecodedIndex getTypeIdx() {
  return Type<ObjT>::idx;
}

inline std::string const& getTypeNameForIdx(DecodedIndex const typeIdx) {
  return getRegisteredNames()[typeIdx];
}

template <typename ObjT>
inline std::string const& getTypeName() {
  return getTypeNameForIdx(getTypeIdx<ObjT>());
}

}}} /* end namespace checkpoint::dispatch::typeregistry */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_TYPE_REGISTRY_H*/
