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

namespace checkpoint { namespace tests { namespace unit {

struct TestSizer : TestHarness { };

struct Test1 {
  int a;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a;
  }
};

struct Test2 {
  int a, b;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a;
    s | b;
  }
};

struct Test3 {
  int a, b, c;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a;
    s | b;
    s | c;
  }
};

struct Test4 {
  int a, b, c;
};

template <typename Serializer>
void serialize(Serializer& s, Test4 t) {
  s | t.a;
  s | t.b;
  s | t.c;
}

TEST_F(TestSizer, test_sizer_1) {
  Test1 t;
  auto const& size = checkpoint::getSize(t);
  EXPECT_EQ(size, sizeof(int));
}

TEST_F(TestSizer, test_sizer_2) {
  Test2 t;
  auto const& size = checkpoint::getSize(t);
  EXPECT_EQ(size, sizeof(int)*2);
}

TEST_F(TestSizer, test_sizer_3) {
  Test3 t;
  auto const& size = checkpoint::getSize(t);
  EXPECT_EQ(size, sizeof(int)*3);
}

TEST_F(TestSizer, test_sizer_4) {
  Test4 t;
  auto const& size = checkpoint::getSize(t);
  EXPECT_EQ(size, sizeof(int)*3);
}

}}} // end namespace checkpoint::tests::unit
