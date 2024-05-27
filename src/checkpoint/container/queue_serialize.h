/*
//@HEADER
// *****************************************************************************
//
//                             queue_serialize.h
//                 DARMA/checkpoint => Serialization Library
//
// Copyright 2020 National Technology & Engineering Solutions of Sandia, LLC
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

#if !defined INCLUDED_CHECKPOINT_CONTAINER_QUEUE_SERIALIZE_H
#define INCLUDED_CHECKPOINT_CONTAINER_QUEUE_SERIALIZE_H

#include "checkpoint/common.h"

#include <queue>
#include <stack>

namespace checkpoint {

template<typename SerializerT, typename T>
void deserializeQueueElems(SerializerT& s, std::queue<T>& q, typename std::queue<T>::size_type size) {
  using Reconstructor =
    dispatch::Reconstructor<typename dispatch::CleanType<T>::CleanT>;

  dispatch::Allocator<T> allocated;
  for (typename std::queue<T>::size_type i = 0; i < size; ++i) {
    auto* reconstructed = Reconstructor::construct(allocated.buf);
    s | *reconstructed;
    q.push(std::move(*reconstructed));
  }
}

template<typename SerializerT, typename T>
void serializeQueueElems(SerializerT& s, std::queue<T> q) {
  while(!q.empty()) {
    s | q.front();
    q.pop();
  }
}

template <
  typename SerializerT,
  typename T,
  typename = std::enable_if_t<
    not std::is_same<
      SerializerT,
      checkpoint::Footprinter
    >::value
  >
>
void serializeQueueLikeContainer(SerializerT& s, std::queue<T>& q) {
  typename std::queue<T>::size_type size = serializeContainerSize(s, q);

  if (s.isUnpacking()) {
    deserializeQueueElems(s, q, size);
  } else {
    serializeQueueElems(s, q);
  }
}

template <typename Serializer, typename T>
void serialize(Serializer& s, std::queue<T>& q) {
  serializeQueueLikeContainer(s, q);
}

template <typename Serializer, typename T>
void serialize(Serializer& s, const std::priority_queue<T>& q) {
  serializeQueueLikeContainer(s, q);
}

template <typename Serializer, typename T>
void serialize(Serializer& s, const std::stack<T>& stack) {
  serializeQueueLikeContainer(s, stack);
}

template <
  typename SerializerT,
  typename Q,
  typename = std::enable_if_t<
    std::is_same<
      SerializerT,
      checkpoint::Footprinter
    >::value
  >
>
void serializeQueueLikeContainer(SerializerT& s, const Q& q) {
  s.countBytes(q);
  s.contiguousBytes(nullptr, sizeof(typename Q::value_type), q.size());
}

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_CONTAINER_QUEUE_SERIALIZE_H*/
