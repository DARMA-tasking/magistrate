/*
//@HEADER
// *****************************************************************************
//
//                          test_virtual_serialize.cc
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

#include "test_harness.h"

#include <checkpoint/checkpoint.h>

#include <gtest/gtest.h>

#include <vector>
#include <memory>

namespace checkpoint { namespace tests { namespace unit {

struct TEST_CONSTRUCT { };

namespace test_1 {

static constexpr std::size_t const num_ints = 1;

enum TestEnum {
  Base, Derived1, Derived2
};

struct TestBase : SerializableBase<TestBase> {
  explicit TestBase(TEST_CONSTRUCT) { init();  }
  explicit TestBase(SERIALIZE_CONSTRUCT_TAG) {}

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | my_ints;
  }

  void init() {
    for (std::size_t i = 0; i < num_ints; i++) {
      my_ints.emplace_back(i*29);
    }
  }

  virtual void check() {
    EXPECT_EQ(my_ints.size(), num_ints);
  }

  virtual TestEnum getID() = 0;

private:
  std::vector<int> my_ints;
};

struct TestDerived1 : SerializableDerived<TestDerived1, TestBase> {
  using Parent = SerializableDerived<TestDerived1, TestBase>;

  explicit TestDerived1(TEST_CONSTRUCT tag) : Parent(tag) { init();  }
  explicit TestDerived1(SERIALIZE_CONSTRUCT_TAG tag) : Parent(tag)  {}

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | my_map;
    s | a;
    s | my_ptr_int;
  }

  void init() {
    for (std::size_t i = 0; i < num_ints; i++) {
      my_map[static_cast<int>(i)] = static_cast<int>(i*29);
    }
    a = 29.;
    my_ptr_int = std::make_unique<int>(139);
  }

  void check() override {
    EXPECT_EQ(my_map.size(), num_ints);
    for (auto&& elm : my_map) {
      EXPECT_EQ(elm.second, elm.first * 29);
    }
    EXPECT_EQ(a, 29.);
    EXPECT_NE(my_ptr_int, nullptr);
    EXPECT_EQ(*my_ptr_int, 139);
    TestBase::check();
  }

  TestEnum getID() override { return TestEnum::Derived1; }

private:
  double a = 0.0;
  std::unordered_map<int, int> my_map;
  std::unique_ptr<int> my_ptr_int;
};

struct TestDerived2 : SerializableDerived<TestDerived2, TestBase> {
  using Parent = SerializableDerived<TestDerived2, TestBase>;

  explicit TestDerived2(TEST_CONSTRUCT tag) : Parent(tag) { init();  }
  explicit TestDerived2(SERIALIZE_CONSTRUCT_TAG tag) : Parent(tag) {}

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | my_set;
  }

  void init() {
    for (std::size_t i = 0; i < num_ints; i++) {
      my_set.insert(static_cast<int>(i));
    }
  }

  void check() override {
    EXPECT_EQ(my_set.size(), num_ints);
    int i = 0;
    for (auto&& elm : my_set) {
      EXPECT_EQ(elm, i++);
    }
    TestBase::check();
  }

  TestEnum getID() override { return TestEnum::Derived2; }

private:
  std::set<int> my_set;
};

static constexpr std::size_t const vec_num_elms = 1;

struct TestWrapper {
  explicit TestWrapper(TEST_CONSTRUCT) { init();  }
  explicit TestWrapper(SERIALIZE_CONSTRUCT_TAG) {}

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | vec;
  }

  void init() {
    for (std::size_t i = 0; i < vec_num_elms; i++) {
      if (i % 2 == 0) {
        vec.emplace_back(std::make_unique<TestDerived1>(TEST_CONSTRUCT{}));
      } else {
        vec.emplace_back(std::make_unique<TestDerived2>(TEST_CONSTRUCT{}));
      }
    }
  }

  void check() {
    EXPECT_EQ(vec.size(), vec_num_elms);
//    int i = 0;
    for (auto&& elm : vec) {
      EXPECT_NE(elm, nullptr);
      elm->check();
      // if (i % 2 == 0) {
      //   EXPECT_EQ(elm->getID(), TestEnum::Derived1);
      // } else {
      //   EXPECT_EQ(elm->getID(), TestEnum::Derived2);
      // }
//      i++;
    }
  }

  std::vector<std::unique_ptr<TestBase>> vec;
};

} /* end namespace test_1 */

template <typename T>
struct TestVirtualSerialize : TestHarness { };

TYPED_TEST_CASE_P(TestVirtualSerialize);

/*
 * General test of serialization/deserialization for input object types
 */

TYPED_TEST_P(TestVirtualSerialize, test_virtual_serialize) {
  using TestType = TypeParam;

  TestType in{TEST_CONSTRUCT{}};
  in.check();

  auto ret = checkpoint::serialize<TestType>(in);
  auto out = checkpoint::deserialize<TestType>(std::move(ret));

  out->check();
}

using ConstructTypes = ::testing::Types<
  test_1::TestWrapper//,
  //test_2::TestWrapper
>;

REGISTER_TYPED_TEST_CASE_P(TestVirtualSerialize, test_virtual_serialize);

INSTANTIATE_TYPED_TEST_CASE_P(
  test_virtual_serialize_inst, TestVirtualSerialize, ConstructTypes
);

}}} // end namespace checkpoint::tests::unit
