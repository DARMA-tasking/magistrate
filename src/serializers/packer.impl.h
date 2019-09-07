/*
//@HEADER
// *****************************************************************************
//
//                                packer.impl.h
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

#include "serdes_common.h"
#include "memory_serializer.h"
#include "buffer/buffer.h"

#include <cstring>

namespace serdes {

template <typename BufferT>
PackerBuffer<BufferT>::PackerBuffer(SerialSizeType const& in_size)
   : MemorySerializer(ModeType::Packing), size_(in_size),
     buffer_(std::make_unique<BufferT>(size_))
{
  MemorySerializer::initializeBuffer(buffer_->getBuffer());
  debug_serdes(
    "PackerBuffer: size=%ld, start_=%p, cur_=%p\n", size_, start_, cur_
  );
}

template <typename BufferT>
PackerBuffer<BufferT>::PackerBuffer(
  SerialSizeType const& in_size, BufferTPtrType buf_ptr
) : MemorySerializer(ModeType::Packing), size_(in_size),
    buffer_(std::move(buf_ptr))
{
  MemorySerializer::initializeBuffer(buffer_->getBuffer());
  debug_serdes(
    "PackerBuffer: size=%ld, start_=%p, cur_=%p\n", size_, start_, cur_
  );
}

template <typename BufferT>
typename PackerBuffer<BufferT>::BufferTPtrType
PackerBuffer<BufferT>::extractPackedBuffer() {
  auto ret = std::move(buffer_);
  buffer_ = nullptr;
  return ret;
}

template <typename BufferT>
void PackerBuffer<BufferT>::contiguousBytes(
  void* ptr, SerialSizeType size, SerialSizeType num_elms
) {
  debug_serdes(
    "PackerBuffer: offset=%ld, size=%ld, num_elms=%ld, ptr=%p, cur_=%p, val=%d\n",
    cur_ - start_, size_, num_elms, ptr, cur_, *reinterpret_cast<int*>(ptr)
  );

  SerialSizeType const len = size * num_elms;
  SerialByteType* spot = this->getSpotIncrement(len);
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunknown-pragmas"
  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
  std::memcpy(spot, ptr, len);
  #pragma GCC diagnostic pop
}

} /* end namespace serdes */
