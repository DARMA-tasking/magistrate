/*
//@HEADER
// *****************************************************************************
//
//                                  unpacker.h
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

#if !defined INCLUDED_CHECKPOINT_SERIALIZERS_UNPACKER_H
#define INCLUDED_CHECKPOINT_SERIALIZERS_UNPACKER_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/memory_serializer.h"
#include "checkpoint/buffer/user_buffer.h"

namespace checkpoint {

template <typename BufferT>
struct UnpackerBuffer : MemorySerializer {
  using BufferPtrType = std::unique_ptr<BufferT>;

  explicit UnpackerBuffer(SerialByteType* buf);

  template <typename... Args>
  explicit UnpackerBuffer(Args&&... args);

  void contiguousBytes(void* ptr, SerialSizeType size, SerialSizeType num_elms);
  SerialSizeType usedBufferSize() const;

private:
  // Size of the actually used memory (for error checking)
  SerialSizeType usedSize_ = 0;

  BufferPtrType buffer_ = nullptr;
};

using Unpacker = UnpackerBuffer<buffer::UserBuffer>;
using UnpackerIO = UnpackerBuffer<buffer::IOBuffer>;

} /* end namespace checkpoint */

#include "checkpoint/serializers/unpacker.impl.h"

#endif /*INCLUDED_CHECKPOINT_SERIALIZERS_UNPACKER_H*/
