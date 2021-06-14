/*
//@HEADER
// *****************************************************************************
//
//                          test_vector_serialize.cc
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

struct VectorTest : TestHarness { };

struct DefCtor {
  static constexpr int val_ctor = 101;

  int i;

  DefCtor() { i = val_ctor; };

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | i;
  }
};

constexpr int DefCtor::val_ctor;

TEST_F(VectorTest, test_vector_default_ctor) {
  std::vector<DefCtor> dcs{DefCtor{}, DefCtor{}, DefCtor{}};
  auto ret = serialize<std::vector<DefCtor>>(dcs);
  auto deser = deserialize<std::vector<DefCtor>>(ret->getBuffer());

  for (auto const& dc : *deser) {
    EXPECT_EQ(dc.i, DefCtor::val_ctor);
  }
}

struct IntrReconstruct {
  static constexpr int val_ctor = 2;
  static constexpr int val_reconstruct = 102;

  int i;

  IntrReconstruct() = delete;
  explicit IntrReconstruct(int ii) : i{ii} { }

  static IntrReconstruct& reconstruct(void* buf) {
    auto b = new (buf) IntrReconstruct{val_reconstruct};
    return *b;
  }

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | i;
  }
};

constexpr int IntrReconstruct::val_ctor;
constexpr int IntrReconstruct::val_reconstruct;

TEST_F(VectorTest, test_vector_intr_reconstruct) {
  std::vector<IntrReconstruct> irs{
    IntrReconstruct{IntrReconstruct::val_ctor},
    IntrReconstruct{IntrReconstruct::val_ctor},
    IntrReconstruct{IntrReconstruct::val_ctor}};
  auto ret = serialize<std::vector<IntrReconstruct>>(irs);
  auto deser = deserialize<std::vector<IntrReconstruct>>(ret->getBuffer());

  for (auto const& ir : *deser) {
    EXPECT_EQ(ir.i, IntrReconstruct::val_ctor);
  }
}

struct NonIntrReconstruct {
  static constexpr int val_ctor = 3;
  static constexpr int val_reconstruct = 103;

  int i;

  NonIntrReconstruct() = delete;
  explicit NonIntrReconstruct(int ii) : i{ii} { }

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | i;
  }
};

constexpr int NonIntrReconstruct::val_ctor;
constexpr int NonIntrReconstruct::val_reconstruct;

void reconstruct(NonIntrReconstruct*& nir, void* buf) {
  nir = new (buf) NonIntrReconstruct{NonIntrReconstruct::val_reconstruct};
}

TEST_F(VectorTest, test_vector_non_intr_reconstruct) {
  std::vector<NonIntrReconstruct> nirs{
    NonIntrReconstruct{NonIntrReconstruct::val_ctor},
    NonIntrReconstruct{NonIntrReconstruct::val_ctor},
    NonIntrReconstruct{NonIntrReconstruct::val_ctor}};
  auto ret = serialize<std::vector<NonIntrReconstruct>>(nirs);
  auto deser = deserialize<std::vector<NonIntrReconstruct>>(ret->getBuffer());

  for (auto const& nir : *deser) {
    EXPECT_EQ(nir.i, NonIntrReconstruct::val_ctor);
  }
}

struct TaggedCtor {
  static constexpr int val_ctor = 4;
  static constexpr int val_tagged_ctor = 104;

  int i;

  TaggedCtor() = delete;
  explicit TaggedCtor(int ii) : i{ii} { }
  explicit TaggedCtor(SERIALIZE_CONSTRUCT_TAG) : i{val_tagged_ctor} { }

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | i;
  }
};

constexpr int TaggedCtor::val_ctor;
constexpr int TaggedCtor::val_tagged_ctor;

TEST_F(VectorTest, test_vector_tagged_ctor) {
  std::vector<TaggedCtor> tcs{
    TaggedCtor{TaggedCtor::val_ctor}, TaggedCtor{TaggedCtor::val_ctor},
    TaggedCtor{TaggedCtor::val_ctor}};
  auto ret = serialize<std::vector<TaggedCtor>>(tcs);
  auto deser = deserialize<std::vector<TaggedCtor>>(ret->getBuffer());

  for (auto const& tc : *deser) {
    EXPECT_EQ(tc.i, TaggedCtor::val_ctor);
  }
}

}}}
