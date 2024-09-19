/*
//@HEADER
// *****************************************************************************
//
//                               test_optional.cc
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

#include <gtest/gtest.h>

#include "test_harness.h"

#include <checkpoint/checkpoint.h>

#include <optional>
#include <string>

namespace checkpoint { namespace tests { namespace unit {

using TestOptional = TestHarness;

template <typename T>
void testOptional(std::optional<T> before) {
    // Test serialization when containing the value
    {
        auto ret = checkpoint::serialize(before);
        auto after = checkpoint::deserialize<std::optional<T>>(ret->getBuffer());

        EXPECT_TRUE(nullptr != after);
        EXPECT_EQ(before.has_value(), after->has_value());
        EXPECT_EQ(before, *after);
    }

    before.reset();

    // Test serialization when there is not value
    {
        auto ret = checkpoint::serialize(before);
        auto after = checkpoint::deserialize<std::optional<T>>(ret->getBuffer());

        EXPECT_TRUE(nullptr != after);
        EXPECT_EQ(before.has_value(), after->has_value());
        EXPECT_EQ(before, *after);
    }
}

TEST_F(TestOptional, test_optional) {
    testOptional<bool>({true});
    testOptional<uint8_t>({1});
    testOptional<uint16_t>({2});
    testOptional<int>({3});
    testOptional<long long>({4});
    testOptional<float>({5});
    testOptional<double>({6});
    testOptional<std::string>({"7"});
}

}}} // end namespace checkpoint::tests::unit
