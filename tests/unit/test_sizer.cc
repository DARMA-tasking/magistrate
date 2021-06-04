/*
//@HEADER
// *****************************************************************************
//
//                                test_sizer.cc
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
#include <checkpoint/dispatch/type_registry.h>

namespace checkpoint { namespace tests { namespace unit {

struct TestSizer : TestHarness { };

struct Test1 {
  int a;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a;
  }
};

TEST_F(TestSizer, test_sizer_1) {
  Test1 t;
  auto const size = checkpoint::getSize(t);

#if defined(SERIALIZATION_ERROR_CHECKING)
  // Expected is
  // sizeof(int) +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test1 +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test1::a +
  auto const expectedSize = sizeof(int) +
    2 * (sizeof(dispatch::typeregistry::DecodedIndex) + sizeof(SerialSizeType));
#else
  // Expected is
  // sizeof(int) +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test1
  auto const expectedSize = sizeof(int) +
    sizeof(dispatch::typeregistry::DecodedIndex) + sizeof(SerialSizeType);
#endif

  EXPECT_EQ(size, expectedSize);
}

struct Test2 {
  int a, b;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a | b;
  }
};

TEST_F(TestSizer, test_sizer_2) {
  Test2 t;
  auto const size = checkpoint::getSize(t);

#if defined(SERIALIZATION_ERROR_CHECKING)
  // Expected is
  // 2 * sizeof(int) +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test2 +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test2::a +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test2::b
  auto const expectedSize = 2 * sizeof(int) +
    3 * (sizeof(dispatch::typeregistry::DecodedIndex) + sizeof(SerialSizeType));
#else
  // Expected is
  // 2 * sizeof(int) +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test2
  auto const expectedSize = 2 * sizeof(int) +
    sizeof(dispatch::typeregistry::DecodedIndex) + sizeof(SerialSizeType);
#endif

  EXPECT_EQ(size, expectedSize);
}

struct Test3 {
  int a, b, c;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a | b | c;
  }
};

TEST_F(TestSizer, test_sizer_3) {
  Test3 t;
  auto const size = checkpoint::getSize(t);

#if defined(SERIALIZATION_ERROR_CHECKING)
  // Expected is
  // 3 * sizeof(int) +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test3 +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test3::a +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test3::b +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test3::c +
  auto const expectedSize = 3 * sizeof(int) +
    4 * (sizeof(dispatch::typeregistry::DecodedIndex) + sizeof(SerialSizeType));
#else
  // Expected is
  // 3 * sizeof(int) +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test3
  auto const expectedSize = 3 * sizeof(int) +
    sizeof(dispatch::typeregistry::DecodedIndex) + sizeof(SerialSizeType);
#endif

  EXPECT_EQ(size, expectedSize);
}

struct Test4 {
  int a, b, c, d;
};

template <typename Serializer>
void serialize(Serializer& s, Test4 t) {
  s | t.a | t.b | t.c | t.d;
}

TEST_F(TestSizer, test_sizer_4) {
  Test4 t;
  auto const size = checkpoint::getSize(t);

#if defined(SERIALIZATION_ERROR_CHECKING)
  // Expected is
  // 4 * sizeof(int) +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test4 +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test4::a +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test4::b +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test4::c +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test4::d
  auto const expectedSize = 4 * sizeof(int) +
    5 * (sizeof(dispatch::typeregistry::DecodedIndex) + sizeof(SerialSizeType));
#else
  // Expected is
  // 4 * sizeof(int) +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test4
  auto const expectedSize = 4 * sizeof(int) +
    sizeof(dispatch::typeregistry::DecodedIndex) + sizeof(SerialSizeType);
#endif

  EXPECT_EQ(size, expectedSize);
}

struct Test5 {
  int a;
  std::array<char, 3> b;
};

template <typename Serializer>
void serialize(Serializer& s, Test5 t) {
  s | t.a | t.b;
}

TEST_F(TestSizer, test_sizer_5) {
  Test5 t;
  auto const size = checkpoint::getSize(t);

#if defined(SERIALIZATION_ERROR_CHECKING)
  // Expected is
  // sizeof(int) + 3 * sizeof(Test5::b::char) +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test5 +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test5::a +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test5::b +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test5::b::char
  auto const expectedSize = sizeof(int) + 3 * sizeof(char) +
    4 * (sizeof(dispatch::typeregistry::DecodedIndex) + sizeof(SerialSizeType));
#else
  // Expected is
  // sizeof(int) + 3 * sizeof(Test5::b::char) +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test5
  auto const expectedSize = sizeof(int) + 3 * sizeof(char) +
    sizeof(dispatch::typeregistry::DecodedIndex) + sizeof(SerialSizeType);
#endif

  EXPECT_EQ(size, expectedSize);
}

struct Test6 {
  double a{3.14};
  std::vector<float> b{1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7};
};

template <typename Serializer>
void serialize(Serializer& s, Test6 t) {
  s | t.a | t.b;
}

TEST_F(TestSizer, test_sizer_6) {
  Test6 t;
  auto const size = checkpoint::getSize(t);

  using VecT = decltype(t.b);
  using CapacityT = decltype(std::declval<VecT>().capacity());
  using SizeT = decltype(std::declval<VecT>().size());

#if defined(SERIALIZATION_ERROR_CHECKING)
  // Expected is
  // sizeof(double) +
  // sizeof(Test6::b::capacity()) + sizeof(Test6::b::size()) +
  // 7 * sizeof(Test6::b::float) +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test6 +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test6::a +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test6::b +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test6::b::capacity() +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test6::b::size() +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test6::b::float
  auto const expectedSize = sizeof(double) + sizeof(CapacityT) + sizeof(SizeT) +
    7 * sizeof(float) +
    6 * (sizeof(dispatch::typeregistry::DecodedIndex) + sizeof(SerialSizeType));
#else
  // Expected is
  // sizeof(double) +
  // sizeof(Test6::b::capacity()) + sizeof(Test6::b::size()) +
  // 7 * sizeof(Test5::b::char) +
  // sizeof(DecodedIndex) + sizeof(SerialSizeType) for Test6
  auto const expectedSize = sizeof(double) + sizeof(CapacityT) + sizeof(SizeT) +
    7 * sizeof(float) + sizeof(dispatch::typeregistry::DecodedIndex) +
    sizeof(SerialSizeType);
#endif

  EXPECT_EQ(size, expectedSize);
}

}}} // end namespace checkpoint::tests::unit
