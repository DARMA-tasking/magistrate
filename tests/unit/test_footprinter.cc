/*
//@HEADER
// *****************************************************************************
//
//                             test_footprinter.cc
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

namespace checkpoint { namespace tests { namespace unit {

struct TestFootprinter : TestHarness { };

struct Test1 {
  template <typename Serializer>
  void serialize(Serializer& s) {
    s | d;
  }

  template <typename Serializer>
  void getMemoryFootprint(Serializer& s) {
    s | d;
  }

  double d;
};

struct Test2 {
  float f;
};

template <typename Serializer>
void getMemoryFootprint(Serializer& s, Test2 t) {
  s | t.f;
}

template <typename Serializer>
void serialize(Serializer& s, Test2 t) {
  s | t.f;
}

TEST_F(TestFootprinter, test_basic_types) {
  int i;
  EXPECT_EQ(checkpoint::getMemoryFootprint(i), sizeof(i));

  double d;
  EXPECT_EQ(checkpoint::getMemoryFootprint(d), sizeof(d));

  {
    int* ptr = nullptr;
    EXPECT_EQ(checkpoint::getMemoryFootprint(ptr), sizeof(ptr));
  }

  {
    int* ptr = new int();
    EXPECT_EQ(checkpoint::getMemoryFootprint(ptr), sizeof(ptr) + sizeof(*ptr));
  }
}

TEST_F(TestFootprinter, test_unique_ptr) {
  {
    std::unique_ptr<Test1> ptr;
    EXPECT_EQ(checkpoint::getMemoryFootprint(ptr), sizeof(ptr));
  }

  {
    auto ptr = std::make_unique<Test1>();
    EXPECT_EQ(checkpoint::getMemoryFootprint(ptr), sizeof(ptr) + sizeof(*ptr));
  }

  {
    auto ptr = std::make_unique<Test2>();
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(ptr),
      sizeof(ptr) + sizeof(*ptr)
    );
  }
}

TEST_F(TestFootprinter, test_string) {
  std::string s = "123456789";
  EXPECT_EQ(
    checkpoint::getMemoryFootprint(s),
    sizeof(s) + s.capacity() * sizeof(s[0]));
}

TEST_F(TestFootprinter, test_vector) {
  {
    std::vector<int> v = {1, 2, 3, 4, 5};
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(v),
      sizeof(v) + v.capacity() * sizeof(int));
  }

  {
    std::vector<Test1*> v = { new Test1(), nullptr };
    EXPECT_EQ(
      checkpoint::getMemoryFootprint(v),
      sizeof(v) + v.capacity() * sizeof(Test1*) + sizeof(*v[0])
    );

    delete v[0];
  }
}

}}} // end namespace checkpoint::tests::unit
