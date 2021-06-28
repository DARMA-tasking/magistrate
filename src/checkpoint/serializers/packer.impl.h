/*
//@HEADER
// *****************************************************************************
//
//                                packer.impl.h
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

#if !defined INCLUDED_CHECKPOINT_SERIALIZERS_PACKER_IMPL_H
#define INCLUDED_CHECKPOINT_SERIALIZERS_PACKER_IMPL_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/memory_serializer.h"
#include "checkpoint/buffer/buffer.h"

#include <cstring>

namespace checkpoint {

template <typename BufferT>
PackerBuffer<BufferT>::PackerBuffer(SerialSizeType const& in_size)
   : MemorySerializer(ModeType::Packing), size_(in_size),
     buffer_(std::make_unique<BufferT>(size_))
{
  MemorySerializer::initializeBuffer(buffer_->getBuffer());

  debug_checkpoint(
    "PackerBuffer: size=%ld, start_=%p, cur_=%p\n",
    size_,
    static_cast<void*>(start_),
    static_cast<void*>(cur_)
  );
}

template <typename BufferT>
PackerBuffer<BufferT>::PackerBuffer(
  SerialSizeType const& in_size, BufferTPtrType buf_ptr
) : MemorySerializer(ModeType::Packing), size_(in_size),
    buffer_(std::move(buf_ptr))
{
  MemorySerializer::initializeBuffer(buffer_->getBuffer());

  debug_checkpoint(
    "PackerBuffer: size=%ld, start_=%p, cur_=%p\n",
    size_,
    static_cast<void*>(start_),
    static_cast<void*>(cur_)
  );
}

template <typename BufferT>
template <typename... Args>
PackerBuffer<BufferT>::PackerBuffer(
  SerialSizeType const& in_size, Args&&... args
) : MemorySerializer(ModeType::Packing),
    size_(in_size),
    buffer_(std::make_unique<BufferT>(std::forward<Args>(args)...))
{
  MemorySerializer::initializeBuffer(buffer_->getBuffer());

  debug_checkpoint(
    "PackerBuffer: size=%ld, start_=%p, cur_=%p\n",
    size_,
    static_cast<void*>(start_),
    static_cast<void*>(cur_)
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
  debug_checkpoint(
    "PackerBuffer: offset=%ld, size=%ld, num_elms=%ld, ptr=%p, cur_=%p\n",
    cur_ - start_, size_, num_elms, ptr, static_cast<void*>(cur_)
  );

  SerialSizeType const len = size * num_elms;
  SerialByteType* spot = this->getSpotIncrement(len);
  #pragma GCC diagnostic push
#if !defined(__has_warning)
  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#elif __has_warning("-Wmaybe-uninitialized")
  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
  std::memcpy(spot, ptr, len);
  #pragma GCC diagnostic pop

  usedSize_ += len;
}

template <typename BufferT>
SerialSizeType PackerBuffer<BufferT>::usedBufferSize() const {
  return usedSize_;
}

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_SERIALIZERS_PACKER_IMPL_H*/
