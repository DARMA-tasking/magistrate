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

static constexpr std::size_t const num_ints = 100;
static constexpr std::size_t const vec_num_elms = 25;

enum TestEnum {
  Base, Derived1, Derived2, Derived3
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace test_1 {

struct TestBase : SerializableBase<TestBase> {
  explicit TestBase(TEST_CONSTRUCT) { init();  }
  explicit TestBase(SERIALIZE_CONSTRUCT_TAG) {}

  virtual ~TestBase() = default;

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
    Parent::check();
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
    Parent::check();
  }

  TestEnum getID() override { return TestEnum::Derived2; }

private:
  std::set<int> my_set;
};

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
    int i = 0;
    for (auto&& elm : vec) {
      EXPECT_NE(elm, nullptr);
      elm->check();
      if (i % 2 == 0) {
        EXPECT_EQ(elm->getID(), TestEnum::Derived1);
      } else {
        EXPECT_EQ(elm->getID(), TestEnum::Derived2);
      }
     i++;
    }
  }

  std::vector<std::unique_ptr<TestBase>> vec;
};

} /* end namespace test_1 */

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace test_2 {

struct TestBase {
  explicit TestBase(TEST_CONSTRUCT) { init();  }
  explicit TestBase(SERIALIZE_CONSTRUCT_TAG) {}

  checkpoint_virtual_serialize_root()

  virtual ~TestBase() = default;

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | a;
  }

  void init() {
    a = 29;
  }

  virtual void check() {
    EXPECT_EQ(a, 29);
  }

  // Return base here to make this variant non-abstract
  virtual TestEnum getID() { return TestEnum::Base; }

private:
  int a = 0;
};

struct TestDerived1 : TestBase {
  using Parent = TestBase;

  explicit TestDerived1(TEST_CONSTRUCT tag) : Parent(tag) { init();  }
  explicit TestDerived1(SERIALIZE_CONSTRUCT_TAG tag) : Parent(tag)  {}

  checkpoint_virtual_serialize_derived_from(TestBase)

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | a;
    s | my_ptr_int;
  }

  void init() {
    a = 29.;
    my_ptr_int = std::make_unique<int>(139);
  }

  void check() override {
    EXPECT_EQ(a, 29.);
    EXPECT_NE(my_ptr_int, nullptr);
    EXPECT_EQ(*my_ptr_int, 139);
    Parent::check();
  }

  TestEnum getID() override { return TestEnum::Derived1; }

private:
  double a = 0.0;
  std::unique_ptr<int> my_ptr_int;
};

struct TestDerived2 : TestBase {
  using Parent = TestBase;

  explicit TestDerived2(TEST_CONSTRUCT tag) : Parent(tag) { init();  }
  explicit TestDerived2(SERIALIZE_CONSTRUCT_TAG tag) : Parent(tag) {}

  checkpoint_virtual_serialize_derived_from(TestBase)

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
    Parent::check();
  }

  TestEnum getID() override { return TestEnum::Derived2; }

private:
  std::set<int> my_set;
};

struct TestDerived3 : TestDerived2 {
  using Parent = TestDerived2;

  explicit TestDerived3(TEST_CONSTRUCT tag) : Parent(tag) { init();  }
  explicit TestDerived3(SERIALIZE_CONSTRUCT_TAG tag) : Parent(tag) {}

  checkpoint_virtual_serialize_derived_from(TestDerived2)

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | x;
  }

  void init() {
    x = 34.;
  }

  void check() override {
    EXPECT_EQ(x, 34.);
    Parent::check();
  }

  TestEnum getID() override { return TestEnum::Derived3; }

private:
  double x = 0.;
};

struct TestWrapper {
  explicit TestWrapper(TEST_CONSTRUCT) { init();  }
  explicit TestWrapper(SERIALIZE_CONSTRUCT_TAG) {}

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    auto vec_size = vec.size();
    s | vec_size;
    vec.resize(vec_size);
    auto vec_derived_size = vec_derived.size();
    s | vec_derived_size;
    vec_derived.resize(vec_derived_size);

    for (auto&& elm : vec) {
      TestBase* base = elm.get();
      checkpoint::allocateConstructForPointer(s, base);
      if (s.isUnpacking()) {
        elm = std::shared_ptr<TestBase>(base);
      }
      s | *elm;
    }

    for (auto&& elm : vec_derived) {
      TestDerived2* derived = elm.get();
      checkpoint::allocateConstructForPointer(s, derived);
      if (s.isUnpacking()) {
        elm = std::shared_ptr<TestDerived2>(derived);
      }
      s | *elm;
    }
  }

  void init() {
    for (std::size_t i = 0; i < vec_num_elms; i++) {
      if (i % 3 == 0) {
        vec.emplace_back(std::make_shared<TestDerived1>(TEST_CONSTRUCT{}));
      } else if (i % 3 == 1) {
        vec.emplace_back(std::make_shared<TestDerived2>(TEST_CONSTRUCT{}));
      } else if (i % 3 == 2) {
        vec.emplace_back(std::make_shared<TestDerived3>(TEST_CONSTRUCT{}));
      }
    }

    for (std::size_t i = 0; i < vec_num_elms; i++) {
      vec_derived.emplace_back(std::make_shared<TestDerived3>(TEST_CONSTRUCT{}));
    }
  }

  void check() {
    EXPECT_EQ(vec.size(), vec_num_elms);
    EXPECT_EQ(vec_derived.size(), vec_num_elms);
    int i = 0;
    for (auto&& elm : vec) {
      EXPECT_NE(elm, nullptr);
      elm->check();
      if (i % 3 == 0) {
        EXPECT_EQ(elm->getID(), TestEnum::Derived1);
      } else if (i % 3 == 1) {
        EXPECT_EQ(elm->getID(), TestEnum::Derived2);
      } else if (i % 3 == 2) {
        EXPECT_EQ(elm->getID(), TestEnum::Derived3);
      }
     i++;
    }
    for (auto&& elm : vec_derived) {
      EXPECT_NE(elm, nullptr);
      elm->check();
      EXPECT_EQ(elm->getID(), TestEnum::Derived3);
    }
  }

  std::vector<std::shared_ptr<TestBase>> vec;
  std::vector<std::shared_ptr<TestDerived2>> vec_derived;
};

} /* end namespace test_2 */

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// This test checks to composability of virtual serialize by composing virtual
// objects from test_1 and test_2 into a third hierarchy.

namespace test_3 {

struct TestBase {
  explicit TestBase(TEST_CONSTRUCT) { init();  }
  explicit TestBase(SERIALIZE_CONSTRUCT_TAG) {}

  checkpoint_virtual_serialize_root()

  virtual ~TestBase() = default;

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | a;
  }

  void init() {
    a = 29;
  }

  virtual void check() {
    EXPECT_EQ(a, 29);
  }

  // Return base here to make this variant non-abstract
  virtual TestEnum getID() { return TestEnum::Base; }

private:
  int a = 0;
};

struct TestDerived1 : TestBase {
  using Parent = TestBase;

  explicit TestDerived1(TEST_CONSTRUCT tag) : Parent(tag) { init();  }
  explicit TestDerived1(SERIALIZE_CONSTRUCT_TAG tag) : Parent(tag)  {}

  checkpoint_virtual_serialize_derived_from(Parent)

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | recur_vec;
  }

  void init() {
    for (std::size_t i = 0; i < vec_num_elms; i++) {
      if (i % 2 == 0) {
        recur_vec.emplace_back(std::make_unique<test_1::TestDerived1>(TEST_CONSTRUCT{}));
      } else {
        recur_vec.emplace_back(std::make_unique<test_1::TestDerived2>(TEST_CONSTRUCT{}));
      }
    }
  }

  void check() override {
    EXPECT_EQ(recur_vec.size(), vec_num_elms);
    int i = 0;
    for (auto&& elm : recur_vec) {
      EXPECT_NE(elm, nullptr);
      elm->check();
      if (i % 2 == 0) {
        EXPECT_EQ(elm->getID(), TestEnum::Derived1);
      } else {
        EXPECT_EQ(elm->getID(), TestEnum::Derived2);
      }
     i++;
    }
    Parent::check();
  }

  TestEnum getID() override { return TestEnum::Derived1; }

private:
  std::vector<std::unique_ptr<test_1::TestBase>> recur_vec;
};

struct TestDerived2 : TestBase {
  using Parent = TestBase;

  explicit TestDerived2(TEST_CONSTRUCT tag) : Parent(tag) { init();  }
  explicit TestDerived2(SERIALIZE_CONSTRUCT_TAG tag) : Parent(tag) {}

  checkpoint_virtual_serialize_derived_from(Parent)

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | recur_vec;
  }

  void init() {
    for (std::size_t i = 0; i < vec_num_elms; i++) {
      if (i % 2 == 0) {
        recur_vec.emplace_back(std::make_unique<test_2::TestDerived1>(TEST_CONSTRUCT{}));
      } else {
        recur_vec.emplace_back(std::make_unique<test_2::TestDerived2>(TEST_CONSTRUCT{}));
      }
    }
  }

  void check() override {
    EXPECT_EQ(recur_vec.size(), vec_num_elms);
    int i = 0;
    for (auto&& elm : recur_vec) {
      EXPECT_NE(elm, nullptr);
      elm->check();
      if (i % 2 == 0) {
        EXPECT_EQ(elm->getID(), TestEnum::Derived1);
      } else {
        EXPECT_EQ(elm->getID(), TestEnum::Derived2);
      }
     i++;
    }
    Parent::check();
  }

  TestEnum getID() override { return TestEnum::Derived2; }

private:
  std::vector<std::unique_ptr<test_2::TestBase>> recur_vec;
};

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
    int i = 0;
    for (auto&& elm : vec) {
      EXPECT_NE(elm, nullptr);
      elm->check();
      if (i % 2 == 0) {
        EXPECT_EQ(elm->getID(), TestEnum::Derived1);
      } else {
        EXPECT_EQ(elm->getID(), TestEnum::Derived2);
      }
     i++;
    }
  }

  std::vector<std::unique_ptr<TestBase>> vec;
};

} /* end namespace test_3 */


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
  test_1::TestWrapper,
  test_2::TestWrapper,
  test_3::TestWrapper
>;

REGISTER_TYPED_TEST_CASE_P(TestVirtualSerialize, test_virtual_serialize);

INSTANTIATE_TYPED_TEST_CASE_P(
  test_virtual_serialize_inst, TestVirtualSerialize, ConstructTypes,
);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Test for virtual serialize with an abstract class

using TestVirtualSerializeTemplated = TestHarness;

struct HolderBase {
  checkpoint_virtual_serialize_root()

  virtual ~HolderBase() = default;

  template <typename Serializer>
  void serialize(Serializer& s) {}
};

template <typename ObjT>
struct HolderObjBase : HolderBase {
  checkpoint_virtual_serialize_derived_from(HolderBase)

  virtual ObjT* get() = 0;

  template <typename Serializer>
  void serialize(Serializer& s) {}
};

template <typename ObjT>
struct HolderBasic final : HolderObjBase<ObjT> {
  checkpoint_virtual_serialize_derived_from(HolderObjBase<ObjT>)

  ObjT* get() override { return obj_; }
  ObjT* obj_ = nullptr;

  template <typename Serializer>
  void serialize(Serializer& s) {}
};

TEST_F(TestVirtualSerializeTemplated, test_virtual_serialize_templated) {
  using TestType = std::unique_ptr<HolderBase>;
  TestType ptr = std::make_unique<HolderBasic<int>>(10);

  auto ret = checkpoint::serialize(ptr);
  checkpoint::deserialize<TestType>(std::move(ret));
}

}}} // end namespace checkpoint::tests::unit
