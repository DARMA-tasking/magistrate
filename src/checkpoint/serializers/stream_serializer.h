/*
//@HEADER
// *****************************************************************************
//
//                             stream_serializer.h
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

#if !defined INCLUDED_CHECKPOINT_SERIALIZERS_STREAM_SERIALIZER_H
#define INCLUDED_CHECKPOINT_SERIALIZERS_STREAM_SERIALIZER_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/base_serializer.h"
#include <ostream>
#include <istream>

namespace checkpoint {

template<typename StreamT = std::ostream>
struct StreamPacker : BaseSerializer {
  StreamPacker(SerialSizeType size, StreamT& m_stream)
    : BaseSerializer(ModeType::Packing), stream(m_stream) {
    //Nothing to do with the size.
    //Pre-allocating a buffer for the stream has more problems than solutions.
  }

  void contiguousBytes(void* ptr, SerialSizeType size, SerialSizeType num_elms) {
    stream.write(static_cast<char*>(ptr), size*num_elms);
    n_bytes += size*num_elms;
  }

  SerialSizeType usedBufferSize() {
    return n_bytes;
  }

private:
  StreamT& stream;
  SerialSizeType n_bytes = 0;
};

template<typename StreamT = std::istream>
struct StreamUnpacker : BaseSerializer {
  StreamUnpacker(StreamT& m_stream)
    : BaseSerializer(ModeType::Unpacking), stream(m_stream) { }

  void contiguousBytes(void* ptr, SerialSizeType size, SerialSizeType num_elms) {
    stream.read(static_cast<char*>(ptr), size*num_elms);
    if(!stream)
      throw std::runtime_error("Stream unable to read required number of bytes!");
    n_bytes += size*num_elms;
  }

  SerialSizeType usedBufferSize() {
    return n_bytes;
  }

private:
  StreamT& stream;
  SerialSizeType n_bytes = 0;
};

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_SERIALIZERS_STREAM_SERIALIZER_H*/
