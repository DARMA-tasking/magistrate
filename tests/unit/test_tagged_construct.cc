/*
//@HEADER
// *****************************************************************************
//
//                           test_tagged_construct.cc
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

#include "test_harness.h"

#include <checkpoint/checkpoint.h>

#include <gtest/gtest.h>

#include <vector>
#include <cstdio>

namespace checkpoint { namespace tests { namespace unit {

template <typename T>
struct TestTaggedConstruct : TestHarness { };

TYPED_TEST_CASE_P(TestTaggedConstruct);

static constexpr int const u_val = 43;

/*
 * Unit test with `UserObjectA` with non-intrusive reconstruct for
 * deserialization purposes
 */

struct UserObjectA {
  UserObjectA(checkpoint::SERIALIZE_CONSTRUCT_TAG) {}
  explicit UserObjectA(int in_u) : u_(in_u) { }

  void check() {
    EXPECT_EQ(u_, u_val);
  }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | u_;
  }

  int u_;
};

void reconstruct(UserObjectA*& obj, void* buf) {
  obj = new (buf) UserObjectA(100);
}

struct UserObjectB;

template <typename SerializerT>
void serialize(SerializerT& s, UserObjectB& x);

struct UserObjectB {
  UserObjectB(checkpoint::SERIALIZE_CONSTRUCT_TAG) {}
  explicit UserObjectB(int in_u) : u_(std::to_string(in_u)) { }

public:
  void check() {
    EXPECT_EQ(u_, std::to_string(u_val));
  }

  template <typename SerializerT>
  friend void serialize(SerializerT&, UserObjectB&);

private:
  std::string u_ = {};
};

template <typename SerializerT>
void serialize(SerializerT& s, UserObjectB& x) {
  s | x.u_;
}

/*
 * General test of serialization/deserialization for input object types
 */

TYPED_TEST_P(TestTaggedConstruct, test_tagged_construct) {
  namespace ser = checkpoint;

  using TestType = TypeParam;

  TestType in(u_val);
  in.check();

  auto ret = ser::serialize<TestType>(in);
  auto out = ser::deserialize<TestType>(std::move(ret));

  out->check();
}

using ConstructTypes = ::testing::Types<
  UserObjectA, UserObjectB
>;

REGISTER_TYPED_TEST_CASE_P(TestTaggedConstruct, test_tagged_construct);
INSTANTIATE_TYPED_TEST_CASE_P(test_tagged, TestTaggedConstruct, ConstructTypes, );

}}} // end namespace checkpoint::tests::unit
