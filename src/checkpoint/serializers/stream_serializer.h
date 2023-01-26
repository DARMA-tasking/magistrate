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

//Used to always static_assert false without compilation issues.
template<class T> struct always_false : std::false_type {};

namespace checkpoint {

template <typename StreamT, eSerializationMode mode, typename... UserTraits>
struct StreamSerializer : Serializer<UserTraits...> {
  using ModeType = eSerializationMode;

  StreamSerializer(ModeType const& in_mode, StreamT& stream)
    : Serializer<UserTraits...>(in_mode), stream(stream)
  { 
    if constexpr ( mode == eSerializationMode::Packing ) {
        start_position = stream.tellp();
    } else if constexpr ( mode == eSerializationMode::Unpacking ){ 
        start_position = stream.tellg();
    } else static_assert(always_false<StreamT>::value, "StreamSerializer can only be used for packing and unpacking");
  }
  
  StreamSerializer(SerialSizeType in_size, ModeType const& in_mode, StreamT& stream) 
    : StreamSerializer(in_mode, stream) {}

  void contiguousBytes(void* ptr, SerialSizeType size, SerialSizeType num_elms){
    SerialSizeType const len = size * num_elms;
    if constexpr ( mode == eSerializationMode::Packing ){ 
        stream.write((char*)ptr, len);
    } else if constexpr ( mode == eSerializationMode::Unpacking ){ 
        stream.read((char*)ptr, len);
    } else static_assert(always_false<StreamT>::value, "StreamSerializer can only be used for packing and unpacking");
  }

  SerialSizeType usedBufferSize() {
    SerialSizeType current_position;

    if constexpr ( mode == eSerializationMode::Packing ){ 
        current_position = static_cast<SerialSizeType>(stream.tellp());
    } else if constexpr ( mode == eSerializationMode::Unpacking ){
        current_position = static_cast<SerialSizeType>(stream.tellg());
    } else static_assert(always_false<StreamT>::value, "StreamSerializer can only be used for packing and unpacking");
    
    return current_position - start_position;
  }

  //TODO: What is the use of this? Is it important for some usecase?
  void extractPackedBuffer(){
    return;
  }

protected:
  StreamT& stream;

  //Initial position of streams, used for calculating in/out size
  SerialSizeType start_position = 0;
};

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_SERIALIZERS_STREAM_SERIALIZER_H*/
