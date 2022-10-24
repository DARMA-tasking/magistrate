/*
//@HEADER
// *****************************************************************************
//
//                       test_kokkos_serialize_pair.cc
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

namespace checkpoint { namespace tests { namespace unit {

struct KokkosPairTest : virtual testing::Test { };

template <typename T1, typename T2>
static void test_kokkos_pair(Kokkos::pair<T1, T2>& refPair) {
    using pairType = Kokkos::pair<T1, T2>;

    auto serialized = checkpoint::serialize<pairType>(refPair);
    auto deserialized = checkpoint::deserialize<pairType>(serialized->getBuffer());
    auto& outPair = *deserialized;

    ASSERT_EQ(refPair.first, outPair.first);
    ASSERT_EQ(refPair.second, outPair.second);
}

TEST_F(KokkosPairTest, test_kokkos_pair) {
    using namespace ::checkpoint;

    auto pairIntInt = Kokkos::pair<int, int>(10, 20);
    test_kokkos_pair(pairIntInt);

    auto pairShortString = Kokkos::pair<short, std::string>(5, "test");
    test_kokkos_pair(pairShortString);

    auto pairUnsignedVector = Kokkos::pair<unsigned, std::vector<int>>{30, {2, 3, 4 ,5}};
    test_kokkos_pair(pairUnsignedVector);

    auto pairIntVoid = Kokkos::pair<int, void>(100);
    test_kokkos_pair(pairIntVoid);
}

}}} // namespace checkpoint::tests::unit

#endif /*KOKKOS_ENABLED_CHECKPOINT*/
