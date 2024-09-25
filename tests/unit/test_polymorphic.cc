/*
//@HEADER
// *****************************************************************************
//
//                             test_polymorphic.cc
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
#include <gtest/gtest.h>

#include "test_harness.h"

#include <checkpoint/checkpoint.h>
#include <checkpoint/dispatch/vrt/base.h>

namespace checkpoint { namespace tests { namespace unit {

using TestPolymorphic = TestHarness;

struct Base {
  explicit Base() = default;
  explicit Base(int val_in): base_val_(val_in) {};
  virtual ~Base() = default;

  checkpoint_virtual_serialize_root()

  int base_val_;
  virtual int getVal() {
    return base_val_;
  }

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | base_val_;
  }
};

struct Derived1: public Base {
  explicit Derived1() = default;
  explicit Derived1(int val_in): Base(0), derived_val_(val_in) {};
  virtual ~Derived1() = default;

  checkpoint_virtual_serialize_derived_from(Base)

  int derived_val_;
  int getVal() override {
    return derived_val_;
  }

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | derived_val_;
  }
};

struct Derived2: public Derived1 {
  explicit Derived2() = default;
  explicit Derived2(int val_in): Derived1(0), derived_val_2_(val_in) {};
  virtual ~Derived2() = default;

  checkpoint_virtual_serialize_derived_from(Derived1)

  int derived_val_2_;
  int getVal() override {
    return derived_val_2_;
  }

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | derived_val_2_;
  }
};

template<typename Base, typename Derived>
void testPolymorphicTypes(int val) {
  std::unique_ptr<Base> task(new Derived(val));
  auto ret = checkpoint::serialize(*task);
  auto out = checkpoint::deserialize<Base>(std::move(ret));

  EXPECT_TRUE(nullptr != out);
  EXPECT_EQ(typeid(*task), typeid(*out));
  EXPECT_TRUE(nullptr != dynamic_cast<Derived*>(out.get()));
  EXPECT_EQ(val, out->getVal());
}

TEST_F(TestPolymorphic, test_polymorphic_type) {
  testPolymorphicTypes<Derived2, Derived2>(5);
  testPolymorphicTypes<Derived1, Derived2>(50);
  testPolymorphicTypes<Base, Derived2>(500);
  testPolymorphicTypes<Derived1, Derived1>(10);
  testPolymorphicTypes<Base, Derived1>(100);
  testPolymorphicTypes<Base, Base>(1);
}

}}} // end namespace checkpoint::tests::unit
