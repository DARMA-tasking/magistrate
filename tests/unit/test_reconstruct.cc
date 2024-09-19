/*
//@HEADER
// *****************************************************************************
//
//                             test_reconstruct.cc
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

#include "test_harness.h"

#include <checkpoint/checkpoint.h>

#include <gtest/gtest.h>

#include <vector>
#include <cstdio>

namespace checkpoint { namespace tests { namespace unit {

template <typename T>
struct TestReconstruct : TestHarness { };

TYPED_TEST_CASE_P(TestReconstruct);

static constexpr int const u_val = 43;
static constexpr int const w_val = 38;

/*
 * Unit test with `UserObjectA` with a default constructor for deserialization
 * purposes
 */

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

/*
 * Unit test with `UserObjectB` with non-intrusive reconstruct for
 * deserialization purposes
 */

struct UserObjectB {
  explicit UserObjectB(int in_u) : u_(in_u) { }

  void check() {
    EXPECT_EQ(u_, u_val);
  }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | u_;
  }

  int u_;
};

void reconstruct(UserObjectB*& obj, void* buf) {
  obj = new (buf) UserObjectB(100);
}

}}} // end namespace checkpoint::tests::unit

/*
 * Unit test with `UserObjectC` with non-intrusive reconstruct for
 * deserialization purposes in the serdes namespace (ADL check)
 */

// Forward-declare UserObjectC
namespace checkpoint { namespace tests { namespace unit {

struct UserObjectC;

template <typename SerializerT>
void serialize(SerializerT& s, UserObjectC& x);

void reconstruct(UserObjectC*& obj, void* buf);

struct UserObjectC {
  explicit UserObjectC(int in_u) : u_(std::to_string(in_u)) { }

public:
  void check() {
    EXPECT_EQ(u_, std::to_string(u_val));
  }

  template <typename SerializerT>
  friend void serialize(SerializerT&, UserObjectC&);

  friend void reconstruct(UserObjectC*&, void*);

private:
  std::string u_ = {};
};

template <typename SerializerT>
void serialize(SerializerT& s, UserObjectC& x) {
  s | x.u_;
}

void reconstruct(UserObjectC*& obj, void* buf) {
  obj = new (buf) UserObjectC(100);
}

}}} // end namespace checkpoint::tests::unit

// Implement the serialize and reconstruct in `checkpoint` namespace
namespace checkpoint {

// This using declaration is only used for convenience
using UserObjType = checkpoint::tests::unit::UserObjectC;


} /* end namespace checkpoint */

namespace checkpoint { namespace tests { namespace unit {

/*
 * Unit test with `UserObjectD` with intrusive reconstruct for deserialization
 * purposes
 */

struct UserObjectD {
  explicit UserObjectD(int in_u) : u_(in_u) { }

  static UserObjectD& reconstruct(void* buf) {
    auto t = new (buf) UserObjectD(100);
    return *t;
  }

  void check() {
    EXPECT_EQ(u_, u_val);
  }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | u_;
  }

  int u_;
};

/*
 * Unit test with `UserObjectE` with a combination of intrusive reconstruct and
 * tagged construction
 */

struct UserObjectBaseE {
  explicit UserObjectBaseE(int in_w) : w_(in_w) { }

  static UserObjectBaseE& reconstruct(void* buf) {
    checkpointAssert(false, "This should never be called");
    auto t = new (buf) UserObjectBaseE(100);
    return *t;
  }

  void check() {
    EXPECT_EQ(w_, w_val);
  }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | w_;
  }

  int w_;
};

struct UserObjectE : UserObjectBaseE {
  UserObjectE(UserObjectBaseE base) : UserObjectBaseE(base) { }

  explicit UserObjectE(int in_u)
    : UserObjectBaseE(w_val), u_(in_u)
  { }
  UserObjectE() : UserObjectBaseE(0) {
    printf("default constructing\n");
  }

  void check() {
    EXPECT_EQ(u_, u_val);
  }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    UserObjectBaseE::serialize(s);
    s | u_;
  }

  int u_;
};

/*
 * General test of serialization/deserialization for input object types
 */

TYPED_TEST_P(TestReconstruct, test_reconstruct_multi_type) {
  namespace ser = checkpoint;

  using TestType = TypeParam;

  TestType in(u_val);
  in.check();

  auto ret = ser::serialize<TestType>(in);
  auto out = ser::deserialize<TestType>(std::move(ret));

  out->check();
}

using ConstructTypes = ::testing::Types<
  UserObjectA,
  UserObjectB,
  UserObjectC,
  UserObjectD,
  UserObjectE
>;

REGISTER_TYPED_TEST_CASE_P(TestReconstruct, test_reconstruct_multi_type);
INSTANTIATE_TYPED_TEST_CASE_P(Test_reconstruct, TestReconstruct, ConstructTypes, );

}}} // end namespace checkpoint::tests::unit
