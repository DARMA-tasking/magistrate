/*
//@HEADER
// *****************************************************************************
//
//                              test_traversal.cc
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

struct TestObject2 {

  TestObject2() = default;

  struct MakeTag { };

  explicit TestObject2(MakeTag)
    : vec1(100)
  { }

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | vec1;
  }

private:
  std::vector<float> vec1;
};

struct TestObject {

  TestObject() = default;

  struct MakeTag { };

  explicit TestObject(MakeTag)
    : obj2(TestObject2::MakeTag{})
  {
    for (int i = 0; i < 10; i++) {
      vec1.push_back(i);
      vec2.push_back(i * 29.34);
    }
  }

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a | b | vec1 | vec2 | obj2;
  }

private:
  int a = 29;
  int b = 36;
  std::vector<int> vec1;
  std::vector<double> vec2;
  TestObject2 obj2;
};

struct TestTraverse : checkpoint::Serializer<> {
  TestTraverse() : checkpoint::Serializer<>(checkpoint::eSerializationMode::None) { }

  void contiguousBytes(void* ptr, std::size_t size, std::size_t num_elms) {
    printf("size=%zu, num=%zu\n", size, num_elms);
    bytes_ += size * num_elms;
  }

  std::size_t bytes_ = 0;
};

int num_vecs = 0;
int total_num_elms = 0;

template <typename SerializerT, typename T>
struct CustomDispatch {
  static void serializeIntrusive(SerializerT& s, T& t) {
    printf("dispatch intrusive\n");
    t.serialize(s);
  }
  static void serializeNonIntrusive(SerializerT& s, T& t) {
    printf("dispatch non-intrusive\n");
    serialize(s, t);
  }
};

template <typename SerializerT, typename U>
struct CustomDispatch<SerializerT, std::vector<U>> {
  static void serializeNonIntrusive(SerializerT& s, std::vector<U>& t) {
    // Do something special here: e.g., an RDMA for the vector during packing
    printf("Traversing vector: size=%zu\n", t.size());
    for (std::size_t i = 0; i < t.size(); i++) {
      printf("\t vector[%zu]=%s", i, std::to_string(t[i]).c_str());
    }
    printf("\n");
    total_num_elms += t.size();
    num_vecs++;
  }
};

struct TestTraverse2 : checkpoint::Serializer<> {
  template <typename U, typename V>
  using DispatcherType = CustomDispatch<U, V>;

  void contiguousBytes(void* ptr, std::size_t size, std::size_t num_elms) { }

  TestTraverse2() : checkpoint::Serializer<>(checkpoint::eSerializationMode::None) { }
};


using TestTraversal = TestHarness;

TEST_F(TestTraversal, test_traversal) {

  using TestType = TestObject;
  TestType t{TestType::MakeTag{}};

  auto traverse = checkpoint::dispatch::Traverse::with<TestObject, TestTraverse>(t);

  constexpr std::size_t si = sizeof(int);
  constexpr std::size_t sd = sizeof(double);
  constexpr std::size_t sf = sizeof(float);
  constexpr std::size_t ss = sizeof(std::size_t);
  EXPECT_EQ(
    traverse.bytes_,
    std::size_t{10*si + 10*sd + si + si + 100*sf + 2*3*ss}
  );
}

TEST_F(TestTraversal, test_traversal_dispatcher) {

  num_vecs = 0;
  total_num_elms = 0;

  using TestType = TestObject;
  TestType t{TestType::MakeTag{}};

  checkpoint::dispatch::Traverse::with<TestObject, TestTraverse2>(t);

  EXPECT_EQ(num_vecs, 3);
  EXPECT_EQ(total_num_elms, 120);
}

}}} // end namespace checkpoint::tests::unit
