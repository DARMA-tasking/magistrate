/*
//@HEADER
// *****************************************************************************
//
//                    test_serialization_error_checking.cc
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

#include "checkpoint/checkpoint.h"
#include "checkpoint/container/map_serialize.h"
#include "test_harness.h"

#include <stdexcept>
#include <vector>
#include <cstdio>
#include <memory>

namespace checkpoint { namespace tests { namespace unit {

struct TestObject : TestHarness { };

struct AA {
  int aa{0};

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | aa;
  }
};

struct BB {
  double bb{1.0};
  AA aa;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | bb | aa;
  }
};

struct CC {
  double cc{2.0};
  BB bb;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | cc | bb;
  }
};

struct DD {
  double dd{3.0};
  CC cc;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | dd | cc;
  }
};

struct EE {
  double ee{4.0};
  DD dd;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | ee | dd;
  }
};

TEST_F(TestObject, test_serialization_error_checking) {
  struct EE ee;
  auto ret = checkpoint::serialize<EE>(ee);
  EXPECT_NO_THROW(checkpoint::deserialize<EE>(ret->getBuffer()));
  EXPECT_THROW(
    checkpoint::deserialize<DD>(ret->getBuffer()), checkpoint::dispatch::serialization_error
  );
}

struct Base {
  checkpoint_virtual_serialize_root()

  virtual ~Base() = default;

  int bb{3};

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | bb;
  }
};

struct Derived : public Base {
  checkpoint_virtual_serialize_derived_from(Base)

  int dd{4};

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | dd;
  }
};

TEST_F(TestObject, test_serialization_error_checking_polymorphic) {
  using BasePtr = std::unique_ptr<Base>;
  using DerivedPtr = std::unique_ptr<Derived>;

  BasePtr const ptr = std::make_unique<Derived>();
  auto ret = checkpoint::serialize(ptr);
  EXPECT_NO_THROW(checkpoint::deserialize<BasePtr>(ret->getBuffer()));
  EXPECT_THROW(
    checkpoint::deserialize<DerivedPtr>(ret->getBuffer()), checkpoint::dispatch::serialization_error
  );
  EXPECT_THROW(
    checkpoint::deserialize<Base>(ret->getBuffer()), checkpoint::dispatch::serialization_error
  );
  EXPECT_THROW(
    checkpoint::deserialize<Derived>(ret->getBuffer()), checkpoint::dispatch::serialization_error
  );
}

}}} // end namespace checkpoint::tests::unit
