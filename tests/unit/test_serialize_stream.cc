/*
//@HEADER
// *****************************************************************************
//
//                           test_serialize_stream.cc
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
#include <fstream>

#include <gtest/gtest.h>

#include <vector>
#include <cstdio>

namespace checkpoint { namespace tests { namespace unit {

template <typename T>
struct TestSerializeStream : TestHarness { };
template <typename T>
struct TestSerializeStreamInPlace : TestHarness { };

TYPED_TEST_CASE_P(TestSerializeStream);
TYPED_TEST_CASE_P(TestSerializeStreamInPlace);

static constexpr int const u_val = 934;

struct UserObjectA {
  UserObjectA() = default;
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

struct UserObjectB {
  UserObjectB() = default;
  explicit UserObjectB(int in_u) : len_(in_u) {
    u_.resize(len_);
    for (int i = 0; i < len_; i++) {
      u_[i] = u_val+i;
    }
  }

  void check() {
    EXPECT_EQ(u_.size(), static_cast<std::size_t>(len_));
    int i = 0;
    for (auto&& elm : u_) {
      EXPECT_EQ(elm, u_val+i++);
    }
  }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | u_;
    s | len_;
  }

  std::vector<double> u_;
  int len_ = 0;
};

struct UserObjectC {
  UserObjectC() = default;
  explicit UserObjectC(int in_u) : u_(std::to_string(in_u)) { }

  void check() {
    EXPECT_EQ(u_, std::to_string(u_val));
  }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | u_;
  }

  std::string u_ = {};
};

/*
 * General test of serialization/deserialization for input object types
 */

TYPED_TEST_P(TestSerializeStream, test_serialize_stream_multi) {
  using TestType = TypeParam;

  TestType in(u_val);
  in.check();

  auto len = checkpoint::getSize(in);
  printf("len=%lu\n", len);
 
  {
    std::ofstream ostream("hello-stream.txt", std::ios::binary | std::ios::out | std::ios::trunc);
    checkpoint::serializeToStream(in, ostream);
  }
  
  {
    std::ifstream istream("hello-stream.txt", std::ios::binary | std::ios::in);
    auto out = checkpoint::deserializeFromStream<TestType>(istream);
    out->check();
  }
}

TYPED_TEST_P(TestSerializeStreamInPlace, test_serialize_stream_multi_in_place) {
  using TestType = TypeParam;

  TestType in(u_val);
  in.check();

  auto len = checkpoint::getSize(in);
  printf("len=%lu\n", len);

  {
    std::ofstream ostream("hello-stream.txt", std::ios::binary | std::ios::out | std::ios::trunc);
    checkpoint::serializeToStream(in, ostream);
  }
  {
    TestType out{};
  
    std::ifstream istream("hello-stream.txt", std::ios::binary | std::ios::in);
    checkpoint::deserializeInPlaceFromStream<TestType>(istream, &out);
  
    out.check();
  }
}

using ConstructTypes = ::testing::Types<
  UserObjectA,
  UserObjectB,
  UserObjectC
>;

REGISTER_TYPED_TEST_CASE_P(TestSerializeStream, test_serialize_stream_multi);
REGISTER_TYPED_TEST_CASE_P(TestSerializeStreamInPlace, test_serialize_stream_multi_in_place);

INSTANTIATE_TYPED_TEST_CASE_P(test_file, TestSerializeStream, ConstructTypes, );
INSTANTIATE_TYPED_TEST_CASE_P(test_file_in_place, TestSerializeStreamInPlace, ConstructTypes, );

}}} // end namespace checkpoint::tests::unit
