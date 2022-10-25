/*
//@HEADER
// *****************************************************************************
//
//           checkpoint_example_polymorphic_macro_nonintrusive.cc
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

/// [Non-Intrusive Serialize polymorphic macro]

#include <checkpoint/checkpoint.h>
#include "checkpoint/dispatch/dispatch_virtual.h"

// \brief Namespace containing types which will be serialized
namespace magistrate { namespace nonintrusive { namespace examples {

// \struct Abstract base class
struct MyBase {

  MyBase() { printf("MyBase cons\n"); }
  explicit MyBase(::checkpoint::SERIALIZE_CONSTRUCT_TAG) { printf("MyBase recons\n"); }

  virtual ~MyBase() = default;

  // Add serializing macro
  checkpoint_virtual_serialize_root()

  int val_ = 0;

  virtual void test() = 0;
};

struct MyObj : public MyBase {

  explicit MyObj(int val) : MyBase() { printf("MyObj cons\n"); val_ = val;}
  explicit MyObj(::checkpoint::SERIALIZE_CONSTRUCT_TAG) {}

  // Add macro for serialization
  checkpoint_virtual_serialize_derived_from(MyBase)

  void test() override {
    printf("test MyObj 10 == %d ?\n", val_);
    assert(val_ == 10);
  }
};

struct MyObj2 : public MyBase {
  explicit MyObj2(int val) { printf("MyObj2 cons\n"); val_=val; }
  explicit MyObj2(::checkpoint::SERIALIZE_CONSTRUCT_TAG) {}

  // Add macro for serialization
  checkpoint_virtual_serialize_derived_from(MyBase)

  void test() override {
    printf("test MyObj2 20 == %d ?\n", val_);
    assert(val_ == 20);
  }
};

struct MyObj3 : public MyBase {

  int a=0, b=0, c=0;

  explicit MyObj3(int val) { printf("MyObj3 cons\n"); a= 10; b=20; c=100; val_=val;}
  explicit MyObj3(::checkpoint::SERIALIZE_CONSTRUCT_TAG) {}

  // Add macro for serialization
  checkpoint_virtual_serialize_derived_from(MyBase)

  void test() override {
    printf("val_ 30  a 10 b 20 c 100 = %d %d %d %d\n", val_, a, b, c);
    assert(val_ == 30);
    assert(a==10);
    assert(b==20);
    assert(c==100);
  }
};

struct ExampleVector {
  std::vector<std::unique_ptr<MyBase>> vec;
};

void test() {

  ExampleVector v;
  v.vec.push_back(std::make_unique<MyObj3>(30));
  v.vec.push_back(std::make_unique<MyObj2>(20));
  v.vec.push_back(std::make_unique<MyObj>(10));

  auto ret = checkpoint::serialize(v);

  {
    // Display information about serialization result
    auto const& buf = ret->getBuffer();
    auto const& buf_size = ret->getSize();
    printf("ptr=%p, size=%ld\n*****\n\n", static_cast<void*>(buf), buf_size);
  }

  auto t = checkpoint::deserialize<ExampleVector>(ret->getBuffer());

  for (auto&& elm : t->vec) {
    elm->test();
  }
}

}}} // end namespace magistrate::nonintrusive::examples

// \brief In Non-Intrusive way, serialize function needs to be placed in the namespace
// of the type which will be serialized.
namespace magistrate { namespace nonintrusive { namespace examples {

template <typename S>
void serialize(S& s, MyBase& obj) {
  s | obj.val_;
  printf("MyBase: serialize val %d\n", obj.val_);
}

template <typename SerializerT>
void serialize(SerializerT& s, MyObj& obj) {
  printf("MyObj: serialize\n");
}

template <typename SerializerT>
void serialize(SerializerT& s, MyObj2& obj) {
  printf("MyObj2: serialize\n");
}

template <typename SerializerT>
void serialize(SerializerT& s, MyObj3& obj) {
  s | obj.a;
  s | obj.b;
  s | obj.c;
  printf("MyObj3: serialize a b c %d %d %d\n", obj.a, obj.b, obj.c);
}

template <typename SerializerT>
void serialize(SerializerT& s, ExampleVector& obj) {
  s | obj.vec;
}

}}} // end namespace magistrate::nonintrusive::examples

int main(int, char**) {
  using namespace magistrate::nonintrusive::examples;

  test();

  return 0;
}

/// [Non-Intrusive Serialize polymorphic macro]

