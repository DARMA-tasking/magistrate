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
#include "checkpoint/dispatch/vrt/object_registry.h"
#include "checkpoint/dispatch/vrt/serializer_registry.h"
#include "checkpoint/dispatch/vrt/inheritance_assert_helpers.h"
#include "checkpoint/dispatch/vrt/serialize_instantiator.h"

#define checkpoint_virtual_serialize_root()                                                      \
  auto _CheckpointVSBaseTypeFn() -> decltype(auto) { return this; }                              \
  virtual void _checkpointDynamicSerialize(                                                      \
    void* s,                                                                                     \
    ::checkpoint::dispatch::vrt::TypeIdx ser_idx,                                                \
    ::checkpoint::dispatch::vrt::TypeIdx expected_idx                                            \
  ) {                                                                                            \
    using _CheckpointBaseType =                                                                  \
      ::checkpoint::dispatch::vrt::checkpoint_base_type_t<decltype(*this)>;                      \
    ::checkpoint::instantiateObjSerializer<                                                      \
      _CheckpointBaseType,                                                                       \
      checkpoint_serializer_variadic_args()                                                      \
    >();                                                                                         \
    ::checkpoint::dispatch::vrt::assertTypeIdxMatch<_CheckpointBaseType>(expected_idx);          \
    auto dispatcher =                                                                            \
      ::checkpoint::dispatch::vrt::serializer_registry::getObjIdx<_CheckpointBaseType>(ser_idx); \
    dispatcher(s, *static_cast<_CheckpointBaseType*>(this));                                     \
  }                                                                                              \
  virtual ::checkpoint::dispatch::vrt::TypeIdx _checkpointDynamicTypeIndex() {                   \
    using _CheckpointBaseType =                                                                  \
      ::checkpoint::dispatch::vrt::checkpoint_base_type_t<decltype(*this)>;                      \
    return ::checkpoint::dispatch::vrt::objregistry::makeObjIdx<_CheckpointBaseType>();          \
  }

#define checkpoint_virtual_serialize_root_decl()                              \
  auto _CheckpointVSBaseTypeFn() -> decltype(auto) { return this; }           \
  virtual void _checkpointDynamicSerialize(                                   \
    void* s,                                                                  \
    ::checkpoint::dispatch::vrt::TypeIdx ser_idx,                             \
    ::checkpoint::dispatch::vrt::TypeIdx expected_idx                         \
  );                                                                          \
  virtual ::checkpoint::dispatch::vrt::TypeIdx _checkpointDynamicTypeIndex();

#define checkpoint_virtual_serialize_root_def(CLASS)                                             \
  void CLASS::_checkpointDynamicSerialize(                                                       \
    void* s,                                                                                     \
    ::checkpoint::dispatch::vrt::TypeIdx ser_idx,                                                \
    ::checkpoint::dispatch::vrt::TypeIdx expected_idx                                            \
  ) {                                                                                            \
    using _CheckpointBaseType =                                                                  \
      ::checkpoint::dispatch::vrt::checkpoint_base_type_t<decltype(*this)>;                      \
    ::checkpoint::instantiateObjSerializer<                                                      \
      _CheckpointBaseType,                                                                       \
      checkpoint_serializer_variadic_args()                                                      \
    >();                                                                                         \
    ::checkpoint::dispatch::vrt::assertTypeIdxMatch<_CheckpointBaseType>(expected_idx);          \
    auto dispatcher =                                                                            \
      ::checkpoint::dispatch::vrt::serializer_registry::getObjIdx<_CheckpointBaseType>(ser_idx); \
    dispatcher(s, *static_cast<_CheckpointBaseType*>(this));                                     \
  }                                                                                              \
  ::checkpoint::dispatch::vrt::TypeIdx CLASS::_checkpointDynamicTypeIndex() {                    \
    using _CheckpointBaseType =                                                                  \
      ::checkpoint::dispatch::vrt::checkpoint_base_type_t<decltype(*this)>;                      \
    return ::checkpoint::dispatch::vrt::objregistry::makeObjIdx<_CheckpointBaseType>();          \
  }

#define checkpoint_virtual_serialize_base(BASE) checkpoint_virtual_serialize_root()

namespace checkpoint { namespace dispatch { namespace vrt {

/**
 * \brief A class at the base of an inheritance hierarchy should inherit from this
 *
 * \param BaseT the base class itself, following CRTP, to provide a
 * common identifier of the whole hierarchy
 */
template <typename BaseT>
struct SerializableBase {
  checkpoint_virtual_serialize_root()
};

}}} /* end namespace checkpoint::dispatch::vrt */

namespace checkpoint {

template <typename BaseT>
using SerializableBase = dispatch::vrt::SerializableBase<BaseT>;

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_VRT_BASE_H*/
