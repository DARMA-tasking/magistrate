/*
//@HEADER
// *****************************************************************************
//
//                  test_kokkos_serialize_complex.cc
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

struct KokkosComplexTest : virtual testing::Test { };

template <typename T1>
static void test_kokkos_complex(Kokkos::complex<T1>& refComplex) {
    using complexType = Kokkos::complex<T1>;

    auto serialized = checkpoint::serialize<complexType>(refComplex);
    auto deserialized = checkpoint::deserialize<complexType>(serialized->getBuffer());
    auto& outComplex = *deserialized;

    ASSERT_DOUBLE_EQ(refComplex.real(), outComplex.real());
    ASSERT_DOUBLE_EQ(refComplex.imag(), outComplex.imag());
}

TEST_F(KokkosComplexTest, test_kokkos_complex) {
    using namespace ::checkpoint;

    auto complexFloat = Kokkos::complex<float>(1.0f, 2.0f);
    test_kokkos_complex(complexFloat);

    auto complexDouble = Kokkos::complex<double>(10.0, 20.0);
    test_kokkos_complex(complexDouble);

    auto complexLongDouble = Kokkos::complex<long double>(100.0l, 200.0l);
    test_kokkos_complex(complexLongDouble);
}

}}} // namespace checkpoint::tests::unit

#endif /*KOKKOS_ENABLED_CHECKPOINT*/
