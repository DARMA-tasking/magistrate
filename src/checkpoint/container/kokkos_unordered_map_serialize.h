/*
//@HEADER
// *****************************************************************************
//
//                       kokkos_unordered_map_serialize.h
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

#if !defined INCLUDED_CHECKPOINT_CONTAINER_KOKKOS_UNORDERED_MAP_SERIALIZE_H
#define INCLUDED_CHECKPOINT_CONTAINER_KOKKOS_UNORDERED_MAP_SERIALIZE_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/serializers_headers.h"
#include "checkpoint/dispatch/allocator.h"
#include "checkpoint/dispatch/dispatch.h"
#include "checkpoint/dispatch/reconstructor.h"

#if MAGISTRATE_ENABLED_KOKKOS

#include <Kokkos_UnorderedMap.hpp>

namespace checkpoint {

template <
  typename Serializer, typename Key, typename Value, typename Device,
  typename Hasher, typename EqualTo
>
void serializeKokkosUnorderedMapElems(
  Serializer& s,
  Kokkos::UnorderedMap<Key, Value, Device, Hasher, EqualTo>& map
) {
  using mapSizeType =
    typename Kokkos::UnorderedMap<Key, Value, Device, Hasher, EqualTo>::size_type;

  for (mapSizeType i = 0; i < map.capacity(); i++) {
    Key keyAtI = map.key_at(i);
    if (map.exists(keyAtI)) {
      Value val = map.value_at(map.find(keyAtI));

      s | keyAtI;
      s | val;
    }
  }
}

template <
  typename Serializer, typename Key, typename Value, typename Device,
  typename Hasher, typename EqualTo
>
void deserializeInsertElems(
  Serializer& s, Kokkos::UnorderedMap<Key, Value, Device, Hasher, EqualTo>& map,
  typename Kokkos::UnorderedMap<Key, Value, Device, Hasher, EqualTo>::size_type
    map_size,
  typename Kokkos::UnorderedMap<Key, Value, Device, Hasher, EqualTo>::size_type
    map_capacity
) {
  using KeyAlloc = dispatch::Allocator<Key>;
  using ValueAlloc = dispatch::Allocator<Value>;
  using KeyReconstructor =
    dispatch::Reconstructor<typename dispatch::CleanType<Key>::CleanT>;
  using ValReconstructor =
    dispatch::Reconstructor<typename dispatch::CleanType<Value>::CleanT>;

  // resize unordered map
  map.rehash(map_capacity);

  KeyAlloc keyAllocated;
  ValueAlloc valAllocated;
  for (SerialSizeType i = 0; i < map_size; i++) {
    auto* key = KeyReconstructor::construct(keyAllocated.buf);
    auto* val = ValReconstructor::construct(valAllocated.buf);

    s | *key;
    s | *val;

    map.insert(std::move(*key), std::move(*val));
  }
}

template <
  typename SerializerT, typename Key, typename Value, typename Device,
  typename Hasher, typename EqualTo
>
typename std::enable_if_t<
  not std::is_same<SerializerT, checkpoint::Footprinter>::value, void
> serialize(
  SerializerT& s,
  Kokkos::UnorderedMap<Key, Value, Device, Hasher, EqualTo>& map
) {
  using UnorderedMapType =
    Kokkos::UnorderedMap<Key, Value, Device, Hasher, EqualTo>;

  typename UnorderedMapType::size_type size = serializeContainerSize(s, map);
  typename UnorderedMapType::size_type capacity = serializeContainerCapacity(s, map);

  if (s.isUnpacking()) {
    deserializeInsertElems(s, map, size, capacity);
  } else {
    serializeKokkosUnorderedMapElems(s, map);
  }
}

template <
  typename SerializerT, typename Key, typename Value, typename Device,
  typename Hasher, typename EqualTo
>
typename std::enable_if_t<
  std::is_same<SerializerT, checkpoint::Footprinter>::value, void
> serialize(
  SerializerT& s,
  Kokkos::UnorderedMap<Key, Value, Device, Hasher, EqualTo>& map
) {
  using UnorderedMapType =
    Kokkos::UnorderedMap<Key, Value, Device, Hasher, EqualTo>;

  typename UnorderedMapType::size_type size = serializeContainerSize(s, map);
  typename UnorderedMapType::size_type capacity = serializeContainerCapacity(s, map);

  s.addBytes(capacity * sizeof(Key) + size * sizeof(Value));
}

} // namespace checkpoint

#endif /*MAGISTRATE_ENABLED_KOKKOS*/

#endif /*INCLUDED_CHECKPOINT_CONTAINER_KOKKOS_UNORDERED_MAP_SERIALIZE_H*/
