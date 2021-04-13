/*
//@HEADER
// *****************************************************************************
//
//                                test_object.cc
//                           DARMA Toolkit v. 1.0.0
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

#include <gtest/gtest.h>

#include "test_harness.h"

#include <checkpoint/checkpoint.h>

#include <vector>
#include <cstdio>

namespace checkpoint { namespace tests { namespace unit {

struct TestObject : TestHarness { };

enum Enum8  : std::uint8_t  { a, b, c, d };
enum Enum64 : std::uint64_t { e, f, g };
enum Enum0                  { h, i, j };

static constexpr int const x_val = 29;
static constexpr int const y_val = 31;
static constexpr int const z_val = 37;
static constexpr int const u_val = 43;
static constexpr int const vec_val = 41;
static constexpr Enum8 const e8_val = c;
static constexpr Enum64 const e64_val = g;
static constexpr Enum0 const e0_val = j;

struct UserObject3 {
  using isByteCopyable = std::true_type;

  int u;

  void init() {
    u = u_val;
  }

  void check() {
    EXPECT_EQ(u, u_val);
  }
};

struct UserObject2 {
  using isByteCopyable = std::false_type;

  int x, y;
  std::vector<int> vec;
  UserObject3 obj;
  Enum8 e8;
  Enum64 e64;
  Enum0 e0;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | x | y | vec | obj | e8 | e64 | e0;
  }

  void init() {
    x = x_val;
    y = y_val;
    vec.push_back(vec_val);
    obj.init();
    e8 = e8_val;
    e64 = e64_val;
    e0 = e0_val;
  }

  void check() {
    EXPECT_EQ(x, x_val);
    EXPECT_EQ(y, y_val);
    EXPECT_EQ(vec.size(), 1UL);
    EXPECT_EQ(vec[0], vec_val);
    obj.check();
    EXPECT_EQ(e8, e8_val);
    EXPECT_EQ(e64, e64_val);
    EXPECT_EQ(e0, e0_val);
  }
};

struct UserObject1 {
  using isByteCopyable = std::false_type;

  int z;
  UserObject2 obj;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | z | obj;
  }

  void init() {
    z = z_val;
    obj.init();
  }

  void check() {
    EXPECT_EQ(z, z_val);
    obj.check();
  }
};

TEST_F(TestObject, test_bytecopy_trait) {
  using namespace ::checkpoint;

  using TestType = UserObject1;
  TestType t;
  t.init();
  t.check();

  auto ret = checkpoint::serialize<TestType>(t);

  auto tptr = checkpoint::deserialize<TestType>(ret->getBuffer());
  auto& t_final = *tptr;

  t_final.check();
}

struct FailureObj1 {
  int a;

  template <typename Serializer> void serialize(Serializer &s) { s | a; }
};

struct FailureObj2 {
  int b;
  FailureObj1 fo1;
  int c;

  template <typename Serializer> void serialize(Serializer &s) {
    s | b | fo1 | c;
  }
};

struct FailureObj3 {
  int d;
  FailureObj2 fo2;
  int e;
  FailureObj1 fo1;
  int f;

  template <typename Serializer> void serialize(Serializer &s) {
    s | d | fo2 | e | fo1 | f;
  }
};

TEST_F(TestObject, test_bytecopy_trait_failure) {
  using namespace ::checkpoint;

  FailureObj1 fo1{1};
  FailureObj2 fo2{2, fo1, 3};
  FailureObj3 fo3{5, fo2, 8, fo1, 13};

  auto ret = checkpoint::serialize<FailureObj3>(fo3);
  auto tptr = checkpoint::deserialize<FailureObj1>(ret->getBuffer());
  EXPECT_NE(tptr->a, 1);
}
}}} // end namespace checkpoint::tests::unit
