/*
//@HEADER
// *****************************************************************************
//
//                                  dispatch.h
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

#if !defined INCLUDED_SERDES_DISPATCH
#define INCLUDED_SERDES_DISPATCH

#include "checkpoint/serdes_common.h"
#include "checkpoint/buffer/buffer.h"
#include "checkpoint/dispatch/dispatch_common.h"
#include "checkpoint/dispatch/dispatch_serializer.h"
#include "checkpoint/dispatch/dispatch_deserializer.h"
#include "checkpoint/dispatch/dispatch_byte_macro.h"

#include <tuple>

namespace serdes {

struct InPlaceTag { };

template <typename T>
struct Dispatch {
  static SerialSizeType sizeType(T& to_size);
  static BufferPtrType packType(
    T& to_pack, SerialSizeType const& size, SerialByteType* buf
  );
  template <typename PackerT>
  static BufferPtrType packTypeWithPacker(
    PackerT& packer, T& to_pack, SerialSizeType const& size
  );
  static T& unpackType(
    SerialByteType* buf, SerialByteType* data, SerialSizeType const& size,
    bool in_place = false
  );

  static SerialSizeType sizeTypePartial(T& to_size);
  static BufferPtrType packTypePartial(
    T& to_pack, SerialSizeType const& size, SerialByteType* buf
  );
  template <typename PackerT>
  static BufferPtrType packTypeWithPackerPartial(
    PackerT& packer, T& to_pack, SerialSizeType const& size
  );
  static T& unpackTypePartial(
    SerialByteType* buf, SerialByteType* data, SerialSizeType const& size
  );
};

template <typename Serializer, typename T>
inline Serializer& operator|(Serializer& s, T& target);

template <typename Serializer, typename T>
inline Serializer& operator&(Serializer& s, T& target);

template <typename Serializer, typename T>
inline void serializeArray(Serializer& s, T* array, SerialSizeType const num_elms);

template <typename Serializer, typename T>
inline void parserdesArray(Serializer& s, T* array, SerialSizeType const num_elms);

template <typename T>
SerializedReturnType serializeType(
  T& to_serialize, BufferObtainFnType fn = nullptr
);

template <typename T>
T* deserializeType(
  SerialByteType* data, SerialSizeType const& size, T* allocBuf = nullptr
);

template <typename T>
void deserializeType(InPlaceTag, SerialByteType* data, SerialSizeType sz, T* t);

template <typename T>
SerializedReturnType serializeTypePartial(
  T& to_serialize, BufferObtainFnType fn = nullptr
);

template <typename T>
T* deserializeTypePartial(
  SerialByteType* data, SerialSizeType const& size, T* allocBuf = nullptr
);

template <typename T>
std::size_t sizeType(T& t);

} /* end namespace serdes */

#include "dispatch.impl.h"

#endif /*INCLUDED_SERDES_DISPATCH*/
