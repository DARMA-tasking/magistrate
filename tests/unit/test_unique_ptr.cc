/*
//@HEADER
// *****************************************************************************
//
//                             test_unique_ptr.cc
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

using TestUniquePtr = TestHarness;

static constexpr int const x_val = 29;
static constexpr int const y_val = 31;
static constexpr int const z_val = 37;
static constexpr int const vec_val = 41;

struct UserObject2 {
  struct MakeTag { };

  UserObject2() = default;

  explicit UserObject2(MakeTag)
    : x(x_val),
      y(y_val)
  {
    vec.push_back(vec_val);
  }

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | x | y | vec;
  }

  void check() {
    EXPECT_EQ(x, x_val);
    EXPECT_EQ(y, y_val);
    EXPECT_EQ(vec.size(), 1UL);
    EXPECT_EQ(vec[0], vec_val);
  }

  int x = 0, y = 0;
  std::vector<int> vec;
};

struct UserObject1 {

  struct MakeTag { };

  UserObject1() = default;
  explicit UserObject1(MakeTag)
    : z(z_val),
      obj(std::make_unique<UserObject2>(UserObject2::MakeTag{}))
  { }

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | z | obj | obj_null | obj_reset_null;
  }

  void check() {
    EXPECT_EQ(z, z_val);
    EXPECT_NE(obj, nullptr);
    obj->check();
    EXPECT_EQ(obj_null, nullptr);
    EXPECT_EQ(obj_reset_null, nullptr);
  }

  int z = 0;
  std::unique_ptr<UserObject2> obj = nullptr;
  std::unique_ptr<UserObject2> obj_null = nullptr;
  std::unique_ptr<UserObject2> obj_reset_null = nullptr;
};

TEST_F(TestUniquePtr, test_unique_ptr_1) {
  UserObject1 t{UserObject1::MakeTag{}};

  auto ret = checkpoint::serialize(t);
  auto out = checkpoint::deserialize<UserObject1>(ret->getBuffer());
  out->check();

  UserObject1 u{UserObject1::MakeTag{}};
  u.obj_reset_null = std::make_unique<UserObject2>();
  u.obj->x = 1;
  u.obj->vec.clear();
  checkpoint::deserializeInPlace(ret->getBuffer(), &u);
  u.check();
}

}}} // end namespace checkpoint::tests::unit
