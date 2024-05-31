/*
//@HEADER
// *****************************************************************************
//
//                          test_variant_serializer.cc
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

#include <variant>
#include <string>

namespace checkpoint { namespace tests { namespace unit {

using VariantTest = TestHarness;

TEST_F(VariantTest, test_variant_basic) {
  std::variant<int, double> v;
  v = 10.0f;

  static_assert(
    checkpoint::SerializableTraits<decltype(v)>::is_bytecopyable,
    "Must be byte copyable"
  );

  auto ret = serialize(v);
  auto deser = deserialize<decltype(v)>(ret->getBuffer());

  EXPECT_EQ(deser->index(), 1);
  EXPECT_EQ(std::get<double>(*deser), 10.0f);
}

TEST_F(VariantTest, test_variant_basic_string) {
  std::variant<int, double, std::string> v;
  v = "test";

  static_assert(
    not checkpoint::SerializableTraits<decltype(v)>::is_bytecopyable,
    "Must not be byte copyable"
  );

  auto ret = serialize(v);
  auto deser = deserialize<decltype(v)>(ret->getBuffer());

  EXPECT_EQ(deser->index(), 2);
  EXPECT_EQ(std::get<std::string>(*deser), "test");
}

struct NotDefaultConstruct {
  NotDefaultConstruct() = delete;
  explicit NotDefaultConstruct(int a) : a_(a) { }
  explicit NotDefaultConstruct(SERIALIZE_CONSTRUCT_TAG) { }
  int a_ = 0;

  template <typename SerializerT>
  void serialize(SerializerT& s) { s | a_; }
};

TEST_F(VariantTest, test_variant_no_default_construct) {
  std::variant<int, double, NotDefaultConstruct, std::string> v;
  v = NotDefaultConstruct{10};

  static_assert(
    not checkpoint::SerializableTraits<decltype(v)>::is_bytecopyable,
    "Must not be byte copyable"
  );

  auto ret = serialize(v);
  auto deser = deserialize<decltype(v)>(ret->getBuffer());

  EXPECT_EQ(deser->index(), 2);
  EXPECT_EQ(std::get<NotDefaultConstruct>(*deser).a_, 10);
}

struct NotCopyConstruct {
  NotCopyConstruct() = delete;
  explicit NotCopyConstruct(int a) : a_(a) { }
  explicit NotCopyConstruct(SERIALIZE_CONSTRUCT_TAG) { }
  NotCopyConstruct(NotCopyConstruct const&) = delete;
  NotCopyConstruct(NotCopyConstruct&&) = default;
  NotCopyConstruct& operator=(NotCopyConstruct&&) = default;
  int a_ = 0;

  template <typename SerializerT>
  void serialize(SerializerT& s) { s | a_; }
};

TEST_F(VariantTest, test_variant_no_copy_construct) {
  std::variant<int, double, NotCopyConstruct, std::string> v;
  v.emplace<NotCopyConstruct>(10);

  static_assert(
    not checkpoint::SerializableTraits<decltype(v)>::is_bytecopyable,
    "Must not be byte copyable"
  );

  auto ret = serialize(v);
  auto deser = deserialize<decltype(v)>(ret->getBuffer());

  EXPECT_EQ(deser->index(), 2);
  EXPECT_EQ(std::get<NotCopyConstruct>(*deser).a_, 10);
}

}}}
