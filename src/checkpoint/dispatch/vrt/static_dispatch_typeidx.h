/*
//@HEADER
// *****************************************************************************
//
//                          static_dispatch_typeidx.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_VRT_STATIC_DISPATCH_TYPEIDX_H
#define INCLUDED_CHECKPOINT_DISPATCH_VRT_STATIC_DISPATCH_TYPEIDX_H

#include "checkpoint/common.h"
#include "checkpoint/dispatch/vrt/registry_common.h"
#include "checkpoint/dispatch/vrt/object_registry.h"

#include <type_traits>

namespace checkpoint { namespace dispatch { namespace vrt {

template <typename T, typename _enabled = void>
struct DispatchTypeIdx;

template <typename T>
struct DispatchTypeIdx<
  T, typename std::enable_if_t<std::is_abstract<T>::value>
> {
  static TypeIdx get() {
    debug_checkpoint("abstract getIndex: %s\n", typeid(T).name());
    return -1;
  }
};

template <typename T>
struct DispatchTypeIdx<
  T, typename std::enable_if_t<not std::is_abstract<T>::value>
> {
  static TypeIdx get() {
    debug_checkpoint("concrete getIndex: %s\n", typeid(T).name());
    return objregistry::makeObjIdx<T>();
  }
};

}}} /* end namespace checkpoint::dispatch::vrt */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_VRT_STATIC_DISPATCH_TYPEIDX_H*/
