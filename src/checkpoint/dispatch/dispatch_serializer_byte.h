/*
//@HEADER
// *****************************************************************************
//
//                          dispatch_serializer_byte.h
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

#if !defined INCLUDED_SRC_CHECKPOINT_DISPATCH_DISPATCH_SERIALIZER_BYTE_H
#define INCLUDED_SRC_CHECKPOINT_DISPATCH_DISPATCH_SERIALIZER_BYTE_H

#include "checkpoint/common.h"
#include "checkpoint/traits/serializable_traits.h"
#include "checkpoint/dispatch/dispatch_serializer_nonbyte.h"

#include <type_traits>
#include <tuple>
#include <cstdlib>

namespace checkpoint { namespace dispatch {

template <typename SerializerT, typename T, typename Dispatcher>
struct SerializerDispatchByte {
  template <typename U>
  using isByteCopyType =
  typename std::enable_if<SerializableTraits<U,void>::is_bytecopyable, T>::type;

  template <typename U>
  using isNotByteCopyType =
  typename std::enable_if<not SerializableTraits<U,void>::is_bytecopyable, T>::type;

  template <typename U = T>
  void operator()(
    SerializerT& s, T* val, SerialSizeType num, isByteCopyType<U>* = nullptr
  ) {
    s.contiguousTyped(s, val, num);
  }

  template <typename U = T>
  void operator()(
    SerializerT& s, T* val, SerialSizeType num, isNotByteCopyType<U>* = nullptr
  ) {
    SerializerDispatchNonByte<SerializerT, T, Dispatcher> dispatch;
    dispatch(s, val, num);
  }
};

}} /* end namespace checkpoint::dispatch */

#endif /*INCLUDED_SRC_CHECKPOINT_DISPATCH_DISPATCH_SERIALIZER_BYTE_H*/
