/*
//@HEADER
// *****************************************************************************
//
//                              test_tuple.cc
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
#include <tuple>

namespace checkpoint { namespace tests { namespace unit {

struct TestTuple : TestHarness { };

struct Base {
  template <typename SerializerT>
  void serialize(SerializerT&) { }

  bool operator==(const Base&) const { return true; }
};
struct DerivedEmpty : Base {
  template <typename SerializerT>
  void serialize(SerializerT&) { }

  bool operator==(const DerivedEmpty&) const { return true; }
};

struct Derived : Base {
  Derived(int in_x) : x(in_x){};
  Derived() : x(1){};
  int x;

  bool operator==(const Derived& rhs) const { return x == rhs.x; }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | x;
  }
};

struct Derived2 : Base {
  Derived2(int in_x) : x(in_x){};
  Derived2() : x(1){};

  Base base;
  int x;

  bool operator==(const Derived2& rhs) const { return x == rhs.x; }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | base;
    s | x;
  }
};

struct Derived3 : Base {
  Derived3(int in_x) : x(in_x){};
  Derived3() : x(1){};

  Derived base;
  int x;

  bool operator==(const Derived3& rhs) const { return x == rhs.x; }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | base;
    s | x;
  }
};

struct Derived4 : Base {
  Derived4(int in_x) : x(in_x){};
  Derived4() : x(1){};

  Derived3 base;
  int x;

  bool operator==(const Derived4& rhs) const { return x == rhs.x; }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | base;
    s | x;
  }
};

template <typename T>
static void testTupleSerialization(std::tuple<T, int> before) {
  // Calculate serialized buffer size for T
  auto first_part = std::get<0>(before);
  auto first_part_size = checkpoint::serialize(first_part)->getSize();

  // calculate serialized buffer size for std::tuple<int>
  auto second_part = std::make_tuple(std::get<1>(before));
  auto second_part_size = checkpoint::serialize(second_part)->getSize();

  // Calculate serialized buffer size for std::tuple<T, int>
  auto ret = checkpoint::serialize(before);
  auto before_size = ret->getSize();

  auto after = checkpoint::deserialize<std::tuple<T, int>>(ret->getBuffer());

  // Check sizes and content of the tuples
  EXPECT_EQ(first_part_size + second_part_size, before_size);
  EXPECT_NE(after, nullptr);
  EXPECT_EQ(sizeof(before), sizeof(*after));
  EXPECT_EQ(std::get<0>(before), std::get<0>(*after));
  EXPECT_EQ(std::get<1>(before), std::get<1>(*after));
}

TEST_F(TestTuple, test_tuple_simple_types) {
  testTupleSerialization<int>(std::make_tuple(123, 0));
  testTupleSerialization<uint8_t>(std::make_tuple(1, 2));
  testTupleSerialization<int64_t>(std::make_tuple(3, 4));
  testTupleSerialization<double>(std::make_tuple(1.0, 2.0));
  testTupleSerialization<float>(std::make_tuple(3.0, 4.0));
}

TEST_F(TestTuple, test_tuple_empty_base_optimization) {
  // Expect Empty Base Optimization to be aplied
  EXPECT_GE(sizeof(Base), 1);
  EXPECT_NE(sizeof(Base) + sizeof(int), sizeof(Derived));

  // Check serialization buffer sizes for the types without tuple
  auto base = Base();
  auto base_size = checkpoint::serialize(base)->getSize();
  auto some_int = 4;
  auto some_int_size = checkpoint::serialize(some_int)->getSize();
  auto derived = Derived(12345);
  auto derived_size = checkpoint::serialize(derived)->getSize();
  EXPECT_EQ(base_size + some_int_size, derived_size);

  testTupleSerialization<Base>(std::make_tuple(Base(), 0));
  testTupleSerialization<DerivedEmpty>(std::make_tuple(DerivedEmpty(), 0));
  testTupleSerialization<Derived>(std::make_tuple(Derived(1), 1));
  testTupleSerialization<Derived2>(std::make_tuple(Derived2(3), 2));
  testTupleSerialization<Derived3>(std::make_tuple(Derived3(5), 3));
  testTupleSerialization<Derived4>(std::make_tuple(Derived4(7), 4));
}

}}} // end namespace checkpoint::tests::unit
