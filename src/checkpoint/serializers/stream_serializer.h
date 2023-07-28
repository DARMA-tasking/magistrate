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

namespace checkpoint {

namespace {
  template <typename StreamT, typename IsPacking>
  struct StreamHolder {};

  template <typename StreamT>
  struct StreamHolder<StreamT, std::true_type> {
    StreamT& stream;

    StreamHolder(StreamT& stream) : stream(stream) {};

    void copy(char* ptr, SerialSizeType len){
      stream.write(ptr, len);
    }

    SerialSizeType position(){
      return stream.tellp();
    }
  };

  template <typename StreamT>
  struct StreamHolder<StreamT, std::false_type> {
    StreamT& stream;

    StreamHolder(StreamT& stream) : stream(stream) {};

    void copy(char* ptr, SerialSizeType len){
      stream.read(ptr, len);
    }

    SerialSizeType position(){
      return stream.tellg();
    }
  };
}

template <typename StreamT, typename IsPacking, typename UserTraits = UserTraitHolder<>>
struct StreamSerializerImpl : Serializer, public UserTraitedType<UserTraits, StreamSerializerImpl, StreamT, IsPacking> {
  using Serializer::ModeType;

  StreamSerializerImpl(ModeType const& in_mode, StreamT& in_stream)
      : Serializer(in_mode), stream(in_stream), stream_start_position(stream.position()) {
    assert(( IsPacking::value && (in_mode == ModeType::Packing)) ||
           (!IsPacking::value && (in_mode == ModeType::Unpacking)));
  }
  
  StreamSerializerImpl(SerialSizeType size, ModeType const& in_mode, StreamT& in_stream) 
      : StreamSerializerImpl(in_mode, in_stream) {}

  void contiguousBytes(void* ptr, SerialSizeType size, SerialSizeType num_elms) {
    stream.copy(static_cast<char*>(ptr), size*num_elms);
  }

  SerialSizeType usedBufferSize() {
    return stream.position() - stream_start_position;
  }

private:
  StreamHolder<StreamT, IsPacking> stream;
  const SerialSizeType stream_start_position;
};

template<typename StreamT = std::ostream, typename... UserTraits>
using StreamPacker = StreamSerializerImpl<StreamT, std::true_type, UserTraits...>;
template<typename StreamT = std::istream, typename... UserTraits>
using StreamUnpacker = StreamSerializerImpl<StreamT, std::false_type, UserTraits...>;

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_SERIALIZERS_STREAM_SERIALIZER_H*/
