/*
//@HEADER
// *****************************************************************************
//
//                  test_kokkos_serialize_unordered_map.cc
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

#if KOKKOS_ENABLED_CHECKPOINT

#include "test_commons.h"
#include "checkpoint/container/kokkos_unordered_map_serialize.h"

namespace checkpoint { namespace tests { namespace unit {

struct KokkosUnorderedMapTest : virtual testing::Test { };

template <
  typename Key, typename Value, typename Device, typename Hasher,
  typename EqualTo
>
static void test_kokkos_unordered_map(
  Kokkos::UnorderedMap<Key, Value, Device, Hasher, EqualTo>& refMap
) {
  using UnorderedMapType =
    Kokkos::UnorderedMap<Key, Value, Device, Hasher, EqualTo>;

  ASSERT_FALSE(refMap.failed_insert());

  auto serialized = checkpoint::serialize<UnorderedMapType>(refMap);
  auto deserialized =
    checkpoint::deserialize<UnorderedMapType>(serialized->getBuffer());
  auto& outMap = *deserialized;

  ASSERT_FALSE(outMap.failed_insert());
  ASSERT_EQ(refMap.size(), outMap.size());

  // check all keys and values
  for (typename UnorderedMapType::size_type i = 0; i < refMap.capacity(); i++) {
    Key refKey = refMap.key_at(i);
    if (refMap.exists(refKey)) {
      ASSERT_TRUE(outMap.exists(refKey));

      Value refVal = refMap.value_at(refMap.find(refKey));
      Value val = outMap.value_at(outMap.find(refKey));
      ASSERT_EQ(refVal, val);
    }
  }
}

TEST_F(KokkosUnorderedMapTest, test_kokkos_unordered_map) {
  int bigSize = 1000;
  auto mapIntIntBig = Kokkos::UnorderedMap<int, int>(bigSize);
  for(int i = 0; i < bigSize; i++) {
    mapIntIntBig.insert(i + bigSize, i * bigSize);
  }
  test_kokkos_unordered_map(mapIntIntBig);

  auto mapIntDouble = Kokkos::UnorderedMap<int, double>(2);
  mapIntDouble.insert(3, 123.34);
  mapIntDouble.insert(4, 10.1112);
  test_kokkos_unordered_map(mapIntDouble);

  auto mapShortString = Kokkos::UnorderedMap<short, std::string>(2);
  mapShortString.insert(5, "123");
  mapShortString.insert(27, "101112");
  test_kokkos_unordered_map(mapShortString);

  auto mapUnsignedVector = Kokkos::UnorderedMap<unsigned, std::vector<int>>(2);
  mapUnsignedVector.insert(11, std::vector<int>{1, 2, 3});
  mapUnsignedVector.insert(22, std::vector<int>{9, 8, 7});
  test_kokkos_unordered_map(mapUnsignedVector);
}

}}} // namespace checkpoint::tests::unit

#endif /*KOKKOS_ENABLED_CHECKPOINT*/
