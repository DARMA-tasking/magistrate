/*
//@HEADER
// *****************************************************************************
//
//                              base_serializer.h
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

#if !defined INCLUDED_CHECKPOINT_SERIALIZERS_BASE_SERIALIZER_H
#define INCLUDED_CHECKPOINT_SERIALIZERS_BASE_SERIALIZER_H

#include "checkpoint/common.h"

#include <type_traits>
#include <cstdlib>

namespace checkpoint {

enum struct eSerializationMode : int8_t {
  None = 0,
  Unpacking = 1,
  Packing = 2,
  Sizing = 3,
  Invalid = -1
};

struct Serializer {
  using ModeType = eSerializationMode;

  explicit Serializer(ModeType const& in_mode) : cur_mode_(in_mode) {}

  ModeType getMode() const { return cur_mode_; }
  bool isSizing() const { return cur_mode_ == ModeType::Sizing; }
  bool isPacking() const { return cur_mode_ == ModeType::Packing; }
  bool isUnpacking() const { return cur_mode_ == ModeType::Unpacking; }

  template <typename SerializerT, typename T>
  void contiguousTyped(SerializerT& serdes, T* ptr, SerialSizeType num_elms) {
    serdes.contiguousBytes(static_cast<void*>(ptr), sizeof(T), num_elms);
  }

  SerialByteType* getBuffer() const { return nullptr; }
  SerialByteType* getSpotIncrement(SerialSizeType const inc) { return nullptr; }

protected:
  ModeType cur_mode_ = ModeType::Invalid;
};

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_SERIALIZERS_BASE_SERIALIZER_H*/
