/*
//@HEADER
// *****************************************************************************
//
//                               unpacker.impl.h
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

#if !defined INCLUDED_CHECKPOINT_SERIALIZERS_UNPACKER_IMPL_H
#define INCLUDED_CHECKPOINT_SERIALIZERS_UNPACKER_IMPL_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/memory_serializer.h"
#include "checkpoint/serializers/unpacker.h"

#include <cstdlib>
#include <cstring>

namespace checkpoint {

template <typename BufferT, typename UserTraits>
UnpackerBuffer<BufferT, UserTraits>::UnpackerBuffer(SerialByteType* buf)
  : MemorySerializer(Serializer::ModeType::Unpacking),
    buffer_(std::make_unique<BufferT>(buf, 0))
{
  MemorySerializer::initializeBuffer(buffer_->getBuffer());

  debug_checkpoint(
    "UnpackerBuffer: start_=%p, cur_=%p\n",
    static_cast<void*>(start_),
    static_cast<void*>(cur_)
  );
}

template <typename BufferT, typename UserTraits>
template <typename... Args>
UnpackerBuffer<BufferT, UserTraits>::UnpackerBuffer(Args&&... args)
  : MemorySerializer(Serializer::ModeType::Unpacking),
    buffer_(std::make_unique<BufferT>(std::forward<Args>(args)...))
{
  MemorySerializer::initializeBuffer(buffer_->getBuffer());

  debug_checkpoint(
    "UnpackerBuffer: start_=%p, cur_=%p\n",
    static_cast<void*>(start_),
    static_cast<void*>(cur_)
  );
}

template <typename BufferT, typename UserTraits>
void UnpackerBuffer<BufferT, UserTraits>::contiguousBytes(
  void* ptr, SerialSizeType size, SerialSizeType num_elms
) {
  debug_checkpoint(
    "UnpackerBuffer: offset=%ld, num_elms=%ld, ptr=%p, cur_=%p\n",
    cur_ - start_, num_elms, ptr, static_cast<void*>(cur_)
  );

  SerialSizeType const len = size * num_elms;
  SerialByteType* spot = this->getSpotIncrement(len);
  std::memcpy(ptr, spot, len);

  usedSize_ += len;
}

template <typename BufferT, typename UserTraits>
SerialSizeType UnpackerBuffer<BufferT, UserTraits>::usedBufferSize() const {
  return usedSize_;
}

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_SERIALIZERS_UNPACKER_IMPL_H*/
