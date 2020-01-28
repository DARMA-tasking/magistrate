/*
//@HEADER
// *****************************************************************************
//
//                         serdes_example_virtual.cc
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

#include "serdes_headers.h"
#include "dispatch/dispatch_virtual.h"

namespace serdes {

  namespace examples {

    struct MyBase : serdes::SerializableBase<MyBase> {
      MyBase() { printf("MyBase cons\n"); }
      explicit MyBase(SERIALIZE_CONSTRUCT_TAG) { printf("MyBase recons\n"); }

      int val_ = 0;

      template <typename S>
      void serialize(S& s) {
	s | val_;
	printf("MyBase: serialize val %d\n", val_);
      }

      virtual void test() = 0;
    };

    struct MyObj : serdes::SerializableDerived<MyObj, MyBase> {
      explicit MyObj(int val) { printf("MyObj cons\n"); val_ = val;}
      explicit MyObj(SERIALIZE_CONSTRUCT_TAG){}

      template <typename SerializerT>
      void serialize(SerializerT& s) {
	printf("MyObj: serialize\n");
      }

      void test() override {
	printf("test MyObj 10 == %d ?\n", val_);
	assert(val_ == 10);
      }
    };

    struct MyObj2 : serdes::SerializableDerived<MyObj2, MyBase> {
      explicit MyObj2(int val) { printf("MyObj2 cons\n"); val_=val; }
      explicit MyObj2(SERIALIZE_CONSTRUCT_TAG) {}

      template <typename SerializerT>
      void serialize(SerializerT& s) {
	printf("MyObj2: serialize\n");
      }
      void test() override {
	printf("test MyObj2 20 == %d ?\n", val_);
	assert(val_ == 20);
      }
    };

    struct MyObj3 : serdes::SerializableDerived<MyObj3, MyBase> {
      int a=0, b=0, c=0;
      explicit MyObj3(int val) { printf("MyObj3 cons\n"); a= 10; b=20; c=100; val_=val;}
      explicit MyObj3(SERIALIZE_CONSTRUCT_TAG) {}

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
     * Example vector that holds virtual elements. User calls
     * `serdes::virtualSerialize(elm, s)` instead of `s | elm`
     */

    struct ExampleVector {

      template <typename SerializerT>
      void serialize(SerializerT& s) {
	std::size_t size = vec.size();
	s | size;
	vec.resize(size);

	for (auto& elm : vec) {
	  serdes::virtualSerialize(elm, s);
	}
      }

      std::vector<MyBase*> vec;
    };

    void test() {

      ExampleVector v;
      v.vec.push_back(new MyObj3(30));
      v.vec.push_back(new MyObj2(20));
      v.vec.push_back(new MyObj(10));

      auto ret = serdes::serializeType<ExampleVector>(v);

      auto const& buf = std::get<0>(ret);
      auto const& buf_size = std::get<1>(ret);

      printf("ptr=%p, size=%ld\n*****\n\n", static_cast<void*>(buf->getBuffer()), buf_size);

      auto tptr = serdes::deserializeType<ExampleVector>(buf->getBuffer(), buf_size);
      auto& t = *tptr;

      for (auto elm : t.vec)
	elm->test();
    }

  }} // end namespace serdes::examples

int main(int, char**) {
  using namespace serdes::examples;

  test();

  return 0;
}
