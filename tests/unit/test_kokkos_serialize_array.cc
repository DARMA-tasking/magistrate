/*
//@HEADER
// *****************************************************************************
//
//                        test_kokkos_serialize_array.cc
//                 DARMA/magistrate => Serialization Library
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

#include <Kokkos_Array.hpp>
#if MAGISTRATE_KOKKOS_ENABLED

#include "test_commons.h"

namespace checkpoint { namespace tests { namespace unit {

struct KokkosArrayTest : virtual testing::Test { };

template <typename T, size_t N>
static void test_kokkos_array(Kokkos::Array<T, N>& refArray) {
    using array_type = Kokkos::Array<T, N>;

    auto serialized = checkpoint::serialize<array_type>(refArray);
    auto deserialized = checkpoint::deserialize<array_type>(serialized->getBuffer());
    auto& outArray = *deserialized;

    for (size_t i = 0; i < N; ++i )
        ASSERT_EQ(refArray[i], outArray[i]);
}

TEST_F(KokkosArrayTest, test_kokkos_array) {
    using namespace ::checkpoint;

    auto arr1 = Kokkos::Array< int, 5 >{ 1, 2, 3, 4, 5 };
    test_kokkos_array(arr1);

    auto arr2 = Kokkos::Array< float, 3 >{ 3.14f, 2.71f, 365.242f };
    test_kokkos_array(arr2);

    auto arr3 = Kokkos::Array< double, 2 >{ 3.14, 2.71 };
    test_kokkos_array(arr3);

    auto arr4 = Kokkos::Array< int, 1 >{ 3 };
    test_kokkos_array(arr4);

    auto empty_arr = Kokkos::Array< double, 0 >{};
    test_kokkos_array(empty_arr);
}

}}} // namespace checkpoint::tests::unit

#endif /*MAGISTRATE_KOKKOS_ENABLED*/
