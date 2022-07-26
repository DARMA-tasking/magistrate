/*
//@HEADER
// *****************************************************************************
//
//                    checkpoint_example_polymorphic.cc
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

/// [Serialize polymorphic]

#include <checkpoint/checkpoint.h>
#include "checkpoint/dispatch/dispatch_virtual.h"

namespace magistrate { namespace intrusive { namespace examples {

struct MyBase : ::checkpoint::SerializableBase<MyBase> {
  MyBase() { printf("MyBase cons\n"); }
  explicit MyBase(::checkpoint::SERIALIZE_CONSTRUCT_TAG) { printf("MyBase recons\n"); }

  virtual ~MyBase() = default;

  int val_ = 0;

  template <typename S>
  void serialize(S& s) {
    s | val_;
    printf("MyBase: serialize val %d\n", val_);
  }

  virtual void test() = 0;
};

struct MyObj : ::checkpoint::SerializableDerived<MyObj, MyBase> {
  explicit MyObj(int val) { printf("MyObj cons\n"); val_ = val;}
  explicit MyObj(::checkpoint::SERIALIZE_CONSTRUCT_TAG){}

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    printf("MyObj: serialize\n");
  }

  void test() override {
    printf("test MyObj 10 == %d ?\n", val_);
    assert(val_ == 10);
  }
};

struct MyObj2 : ::checkpoint::SerializableDerived<MyObj2, MyBase> {
  explicit MyObj2(int val) { printf("MyObj2 cons\n"); val_=val; }
  explicit MyObj2(::checkpoint::SERIALIZE_CONSTRUCT_TAG) {}

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    printf("MyObj2: serialize\n");
  }
  void test() override {
    printf("test MyObj2 20 == %d ?\n", val_);
    assert(val_ == 20);
  }
};

struct MyObj3 : ::checkpoint::SerializableDerived<MyObj3, MyBase> {
  int a=0, b=0, c=0;
  explicit MyObj3(int val) { printf("MyObj3 cons\n"); a= 10; b=20; c=100; val_=val;}
  explicit MyObj3(::checkpoint::SERIALIZE_CONSTRUCT_TAG) {}

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s|a|b|c;
    printf("MyObj3: serialize a b c %d %d %d\n", a, b, c);
  }
  void test() override {
    printf("val_ 30  a 10 b 20 c 100 = %d %d %d %d\n", val_, a, b, c);
    assert(val_ == 30);
    assert(a==10);
    assert(b==20);
    assert(c==100);
  }
};

/*
 * Example vector that holds a vector of unique_ptr to MyBase
 */

struct ExampleVector {

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | vec;
  }

  std::vector<std::unique_ptr<MyBase>> vec;
};

void test() {

  ExampleVector v;
  v.vec.push_back(std::make_unique<MyObj3>(30));
  v.vec.push_back(std::make_unique<MyObj2>(20));
  v.vec.push_back(std::make_unique<MyObj>(10));

  auto ret = checkpoint::serialize(v);

  auto const& buf = ret->getBuffer();
  auto const& buf_size = ret->getSize();

  printf("ptr=%p, size=%ld\n*****\n\n", static_cast<void*>(buf), buf_size);

  auto t = checkpoint::deserialize<ExampleVector>(buf);

  for (auto&& elm : t->vec) {
    elm->test();
  }
}

}}} // end namespace magistrate::intrusive::examples

int main(int, char**) {
  using namespace magistrate::intrusive::examples;

  test();

  return 0;
}

/// [Serialize polymorphic]
