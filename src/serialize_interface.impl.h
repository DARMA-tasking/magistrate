/*
//@HEADER
// ************************************************************************
//
//                          serialize_interface.impl.h
//                           checkpoint
//              Copyright (C) 2017 NTESS, LLC
//
// Under the terms of Contract DE-NA-0003525 with NTESS, LLC,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// ************************************************************************
//@HEADER
*/

#if !defined INCLUDED_SERIALIZE_INTERFACE_IMPL
#define INCLUDED_SERIALIZE_INTERFACE_IMPL

#include "serdes_common.h"
#include "serdes_headers.h"
#include "serialize_interface.h"
#include "buffer/buffer.h"

#include <memory>

namespace serialization { namespace interface {

template <typename T>
SerializedReturnType serialize(T& target, BufferCallbackType fn) {
  auto ret = ::serdes::serializeType<T>(target, fn);
  auto& buf = std::get<0>(ret);
  std::unique_ptr<SerializedInfo> base_ptr(
    static_cast<SerializedInfo*>(buf.release())
  );
  return base_ptr;
}

template <typename T>
T* deserialize(SerialByteType* buf, SizeType size, T* user_buf) {
  return ::serdes::deserializeType<T>(buf, size, user_buf);
}

template <typename T>
T* deserialize(SerializedReturnType&& in) {
  return ::serdes::deserializeType<T>(in->getBuffer(), in->getSize());
}

template <typename T>
SerializedReturnType serializePartial(T& target, BufferCallbackType fn) {
  auto ret = ::serdes::serializeTypePartial<T>(target, fn);
  auto& buf = std::get<0>(ret);
  std::unique_ptr<SerializedInfo> base_ptr(
    static_cast<SerializedInfo*>(buf.release())
  );
  return base_ptr;
}

template <typename T>
T* deserializePartial(SerialByteType* buf, SizeType size, T* user_buf) {
  return ::serdes::deserializeTypePartial<T>(buf, size, user_buf);
}

}} /* end namespace serialization::interface */

#endif /*INCLUDED_SERIALIZE_INTERFACE_IMPL*/
