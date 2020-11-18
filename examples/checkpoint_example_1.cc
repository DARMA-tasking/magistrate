/*
//@HEADER
// *****************************************************************************
//
//                           checkpoint_example_1.cc
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

/// [Serialize structure built-in types]

#include <checkpoint/checkpoint.h>

#include <cstdio>

namespace checkpoint { namespace examples {

// \struct MyTest
// \brief Simple structure with two variables of built-in types
struct MyTest {
  int a = 29, b = 31;

  // \brief Default constructor
  //
  // The default constructor is needed for the (de)serialization.
  // (required for serialization)
  MyTest() = default;

  // \brief Constructor with two parameters
  //
  // \param[in] Initial value for `a`
  // \param[in] Initial value for `b`
  //
  MyTest(int ai, int bi) : a(ai), b(bi) { };

  /// \brief Printing function unto the standard display
  void print() {
    printf("MyTest: a=%d, b=%d\n", a, b);
  }

  // \brief Templated function for serializing/deserializing
  // a variable of type `MyTest`
  //
  // \tparam <Serializer> { Type for storing the serialized result }
  // \param[in,out] Variable for storing the serialized result
  //
  // \note The routine `serialize` is actually a two-way routine:
  // - it creates the serialized result `s` by combining
  //   the variables `a` and `b`; this creation phase is run
  //   when the status of the serializer `s` is `Packing`.
  // - it can extract from a serialized result `s` the values
  //   to place in the variables `a` and `b`; this extraction phase
  //   is run when the status of the serializer `s` is `Unpacking`.
  //
  template <typename Serializer>
  void serialize(Serializer& s) {
    printf("MyTest serialize\n");
    //
    // a = variable of type `int` (built-in type)
    // The serialization / deserialization of a built-in type
    // is obtained directly by using the pipe operator.
    //
    s | a;
    //
    // b = variable of type `int` (built-in type)
    // The serialization / deserialization of a built-in type
    // is obtained directly by using the pipe operator.
    //
    s | b;
  }
};

}} // end namespace checkpoint::examples

int main(int, char**) {
  using namespace checkpoint::examples;

  // Define a variable of custom type `MyTest`
  MyTest my_test_inst(11, 12);
  my_test_inst.print();

  // Call the serialization routine for the variable `my_test_inst`
  // The output is a unique pointer: `std::unique_ptr<SerializedInfo>`
  // (defined in `src/checkpoint_api.h`)
  auto ret = checkpoint::serialize(my_test_inst);

  {
    // Display information about the serialization "message"
    auto const& buf = ret->getBuffer();
    auto const& buf_size = ret->getSize();
    printf("ptr=%p, size=%ld\n", static_cast<void*>(buf), buf_size);
  }

  // De-serialization call to create a new pointer `t` of type `MyTest*`
  auto t = checkpoint::deserialize<MyTest>(ret->getBuffer());

  // Display the result
  t->print();

  return 0;
}

/// [Serialize structure built-in types]
