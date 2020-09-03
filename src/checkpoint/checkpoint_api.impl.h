/*
//@HEADER
// *****************************************************************************
//
//                            checkpoint_api.impl.h
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

#if !defined INCLUDED_CHECKPOINT_CHECKPOINT_API_IMPL_H
#define INCLUDED_CHECKPOINT_CHECKPOINT_API_IMPL_H

#include "checkpoint/common.h"
#include <checkpoint/checkpoint.h>
#include "checkpoint/checkpoint_api.h"
#include "buffer/buffer.h"

#include <memory>

namespace checkpoint {

template <typename T>
SerializedReturnType serialize(T& target, BufferCallbackType fn) {
  auto ret = dispatch::serializeType<T>(target, fn);
  auto& buf = std::get<0>(ret);
  std::unique_ptr<SerializedInfo> base_ptr(
    static_cast<SerializedInfo*>(buf.release())
  );
  return base_ptr;
}

template <typename T>
T* deserialize(char* buf, char* object_buf) {
  return dispatch::deserializeType<T>(buf, object_buf);
}

template <typename T>
std::unique_ptr<T> deserialize(char* buf) {
  auto t = dispatch::deserializeType<T>(buf);
  return std::unique_ptr<T>(t);
}

template <typename T>
std::unique_ptr<T> deserialize(SerializedReturnType&& in) {
  auto t = dispatch::deserializeType<T>(in->getBuffer());
  return std::unique_ptr<T>(t);
}

template <typename T>
void deserializeInPlace(char* buf, T* t) {
  return dispatch::deserializeType<T>(dispatch::InPlaceTag{}, buf, t);
}

template <typename T>
std::size_t getSize(T& target) {
  return dispatch::Standard::size<T, Sizer>(target);
}

template <typename T>
std::size_t getMemoryFootprint(T& target) {
  return dispatch::Standard::footprint<T, Footprinter>(target);
}

template <typename T>
void serializeToFile(T& target, std::string const& file) {
  auto len = getSize<T>(target);
  dispatch::Standard::pack<T, PackerBuffer<buffer::IOBuffer>>(
    target, len, buffer::IOBuffer::WriteToFileTag{}, len, file
  );
}

template <typename T>
std::unique_ptr<T> deserializeFromFile(std::string const& file) {
  auto mem = dispatch::Standard::allocate<T>();
  T* t_buf = dispatch::Standard::construct<T>(mem);
  auto t = dispatch::Standard::unpack<T, UnpackerBuffer<buffer::IOBuffer>>(
    t_buf, buffer::IOBuffer::ReadFromFileTag{}, file
  );
  return std::unique_ptr<T>(t);
}

template <typename T>
void deserializeInPlaceFromFile(std::string const& file, T* t) {
  dispatch::Standard::unpack<T, UnpackerBuffer<buffer::IOBuffer>>(
    t, buffer::IOBuffer::ReadFromFileTag{}, file
  );
}

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_CHECKPOINT_API_IMPL_H*/
