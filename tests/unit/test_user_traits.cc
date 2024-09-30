/*
//@HEADER
// *****************************************************************************
//
//                             test_user_traits.cc
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
struct TestUserTraits : TestHarness { };
TYPED_TEST_CASE_P(TestUserTraits);

struct CheckpointTrait {};
struct CheckpointTraitNonintrusive {};
namespace CheckpointNamespace {
struct CheckpointTraitNamespaced {};
}

struct ShallowTrait {};
struct TraitPairA {};
struct TraitPairB {};

static constexpr int const u_val = 'a';
struct UserObjectA {
  UserObjectA() = default;
  explicit UserObjectA(int val)
    : name('A'), val_a(val) {};
  explicit UserObjectA(int val, char name_)
    : name(name_), val_a(val) {};

  template <typename S>
  void serialize(S& s) {
    std::cout << "A: serializing with type "
      << abi::__cxa_demangle(typeid(s).name(), nullptr, nullptr, nullptr)
      << std::endl;
    EXPECT_FALSE((S::template has_traits<ShallowTrait>::value));

    EXPECT_EQ(
      (S::template has_traits<TraitPairA>::value),
      (S::template has_traits<TraitPairB>::value)
    );

    if constexpr(S::template has_traits<CheckpointTrait>::value){
      s | name;
    }
    s | val_a;
  }

  char name;
  int val_a;
};

template<
  typename S,
  typename = typename S::template has_traits<CheckpointTraitNonintrusive>::type
>
void serialize(S& s, UserObjectA& obj){
  s | obj.name;
  obj.serialize(s);
}

namespace CheckpointNamespace {
template<
  typename S,
  typename = typename S::template has_traits<CheckpointTraitNamespaced>::type
>
void serialize(S& s, UserObjectA& obj){
  s | obj.name;
  obj.serialize(s);
}
} // CheckpointNamespace

/*
 * Test that trait handling works from different sources
 */
TYPED_TEST_P(TestUserTraits, test_serialize_extra) {
  using Trait = TypeParam;

  UserObjectA objA(u_val);
  char new_name = 'Z', old_name = objA.name;
  ASSERT_NE(new_name, old_name);

  //Without the checkpoint trait, name shouldn't be checkpointed.
  auto serA = checkpoint::serialize(objA);
  objA.name = new_name;
  checkpoint::deserializeInPlace(serA->getBuffer(), &objA);
  EXPECT_EQ(objA.name, new_name);
  EXPECT_NE(objA.name, old_name);

  //With the checkpoint trait, name change should be undone
  objA.name = old_name;
  serA = checkpoint::serialize<UserObjectA, Trait>(objA);
  objA.name = new_name;
  checkpoint::deserializeInPlace<UserObjectA, Trait>(serA->getBuffer(), &objA);
  EXPECT_EQ(objA.name, old_name);
  EXPECT_NE(objA.name, new_name);
}
REGISTER_TYPED_TEST_CASE_P(TestUserTraits, test_serialize_extra);
using TraitTypes = ::testing::Types<CheckpointTrait, CheckpointTraitNonintrusive, CheckpointNamespace::CheckpointTraitNamespaced>;
INSTANTIATE_TYPED_TEST_CASE_P(checkpoint_trait, TestUserTraits, TraitTypes, );



struct UserObjectB : public UserObjectA {
  UserObjectB() = default;
  explicit UserObjectB(int val)
    : UserObjectA(0, 'B'), val_b(val) {};

  template <typename S>
  void serialize(S& s) {
    std::cout << "B: serializing with type "
      << abi::__cxa_demangle(typeid(s).name(), nullptr, nullptr, nullptr)
      << std::endl;
    auto new_s = s.template withoutTraits<ShallowTrait>();
    if (S::template has_traits<TraitPairA>::value){
      auto newer_s = new_s.template withTraits<TraitPairB>();
      UserObjectA::serialize(newer_s);
    } else {
      UserObjectA::serialize(new_s);
    }

    s | val_b;
  }

  int val_b;
};

TEST(TestUserTraits, test_trait_removal) {
  UserObjectB objB(u_val);
  int old_b_val = objB.val_b, old_a_val = objB.val_a;
  int new_b_val = old_b_val+1, new_a_val = old_a_val+1;

  auto serB = checkpoint::serialize<UserObjectB, ShallowTrait>(objB);
  objB.val_b = new_b_val;
  objB.val_a = new_a_val;
  checkpoint::deserializeInPlace<UserObjectB, ShallowTrait>(serB->getBuffer(), &objB);
  EXPECT_EQ(objB.val_b, old_b_val);
  EXPECT_NE(objB.val_b, new_b_val);
  EXPECT_EQ(objB.val_a, old_a_val);
  EXPECT_NE(objB.val_a, new_a_val);
}

TEST(TestUserTraits, test_trait_addition) {
  UserObjectB objB(u_val);
  int old_b_val = objB.val_b, old_a_val = objB.val_a;
  int new_b_val = old_b_val+1, new_a_val = old_a_val+1;

  auto serB = checkpoint::serialize<UserObjectB, TraitPairA>(objB);
  objB.val_b = new_b_val;
  objB.val_a = new_a_val;
  checkpoint::deserializeInPlace<UserObjectB, TraitPairA>(serB->getBuffer(), &objB);
  EXPECT_EQ(objB.val_b, old_b_val);
  EXPECT_NE(objB.val_b, new_b_val);
  EXPECT_EQ(objB.val_a, old_a_val);
  EXPECT_NE(objB.val_a, new_a_val);
}


}}} // end namespace checkpoint::tests::unit
