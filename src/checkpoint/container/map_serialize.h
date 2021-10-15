/*
//@HEADER
// *****************************************************************************
//
//                               map_serialize.h
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

#if !defined INCLUDED_CHECKPOINT_CONTAINER_MAP_SERIALIZE_H
#define INCLUDED_CHECKPOINT_CONTAINER_MAP_SERIALIZE_H

#include "checkpoint/common.h"
#include "checkpoint/dispatch/allocator.h"
#include "checkpoint/serializers/serializers_headers.h"
#include "checkpoint/container/container_serialize.h"

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

namespace checkpoint {

template <typename Serializer, typename ContainerT, typename ElmT>
inline typename std::enable_if_t<
  not std::is_same<Serializer, checkpoint::Footprinter>::value,
  void
> deserializeEmplaceElems(
  Serializer& s, ContainerT& cont, typename ContainerT::size_type size
) {
  using Alloc = dispatch::Allocator<ElmT>;
  using Reconstructor =
    dispatch::Reconstructor<typename dispatch::CleanType<ElmT>::CleanT>;

  Alloc allocated;
  for (typename ContainerT::size_type i = 0; i < size; i++) {
    auto* reconstructed = Reconstructor::construct(allocated.buf);
    s | *reconstructed;
    cont.emplace(std::move(*reconstructed));
  }
}

template <typename Serializer, typename ContainerT, typename ElmT>
inline typename std::enable_if_t<
  std::is_same<Serializer, checkpoint::Footprinter>::value,
  void
> deserializeEmplaceElems(
  Serializer& s, ContainerT& cont, typename ContainerT::size_type size
) { }

template <typename Serializer, typename ContainerT>
inline void serializeMapLikeContainer(Serializer& s, ContainerT& cont) {
  using ValueT = typename ContainerT::value_type;

  typename ContainerT::size_type size = serializeContainerSize(s, cont);

  if (s.isUnpacking()) {
    deserializeEmplaceElems<Serializer, ContainerT, ValueT>(s, cont, size);
  } else {
    serializeContainerElems<Serializer, ContainerT>(s, cont);
  }
}

template <typename Serializer, typename T, typename U, typename Comp>
inline void serialize(Serializer& s, std::map<T, U, Comp>& map) {
  serializeMapLikeContainer(s, map);
}

template <typename Serializer, typename T, typename U, typename Comp>
inline void serialize(Serializer& s, std::multimap<T, U, Comp>& map) {
  serializeMapLikeContainer(s, map);
}

template <typename Serializer, typename T, typename Comp>
inline void serialize(Serializer& s, std::set<T, Comp>& set) {
  serializeMapLikeContainer(s, set);
}

template <typename Serializer, typename T, typename Comp>
inline void serialize(Serializer& s, std::multiset<T, Comp>& set) {
  serializeMapLikeContainer(s, set);
}

template <typename Serializer, typename T, typename U, typename Hash, typename Eq>
inline void serialize(Serializer& s, std::unordered_map<T, U, Hash, Eq>& map) {
  serializeMapLikeContainer(s, map);
}

template <typename Serializer, typename T, typename U, typename Hash, typename Eq>
inline void serialize(Serializer& s, std::unordered_multimap<T, U, Hash, Eq>& map) {
  serializeMapLikeContainer(s, map);
}

template <typename Serializer, typename T, typename Hash, typename Eq>
inline void serialize(Serializer& s, std::unordered_set<T, Hash, Eq>& set) {
  serializeMapLikeContainer(s, set);
}

template <typename Serializer, typename T, typename Hash, typename Eq>
inline void serialize(Serializer& s, std::unordered_multiset<T, Hash, Eq>& set) {
  serializeMapLikeContainer(s, set);
}

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_CONTAINER_MAP_SERIALIZE_H*/
