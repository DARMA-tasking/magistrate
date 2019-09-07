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

#include "serdes_headers.h"
#include "serialization_library_headers.h"

#include <vector>
#include <cstdio>

namespace serdes { namespace tests { namespace unit {

struct TestObject : TestHarness { };

static constexpr int const x_val = 29;
static constexpr int const y_val = 31;
static constexpr int const z_val = 37;
static constexpr int const u_val = 43;
static constexpr int const vec_val = 41;

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

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | x | y | vec | obj;
  }

  void init() {
    x = x_val;
    y = y_val;
    vec.push_back(vec_val);
    obj.init();
  }

  void check() {
    EXPECT_EQ(x, x_val);
    EXPECT_EQ(y, y_val);
    EXPECT_EQ(vec.size(), 1UL);
    EXPECT_EQ(vec[0], vec_val);
    obj.init();
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
  using namespace ::serialization::interface;

  using TestType = UserObject1;
  TestType t;
  t.init();
  t.check();

  auto ret = serialization::interface::serialize<TestType>(t);

  auto tptr = serialization::interface::deserialize<TestType>(
    ret->getBuffer(), ret->getSize()
  );
  auto& t_final = *tptr;

  t_final.check();
}

}}} // end namespace serdes::tests::unit
