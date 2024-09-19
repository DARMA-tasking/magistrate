/*
//@HEADER
// *****************************************************************************
//
//                           checkpoint_example_2.cc
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

/// [Serialize custom structure]

#include <checkpoint/checkpoint.h>

#include <cstdio>

namespace checkpoint { namespace intrusive { namespace examples {

// \struct MyTest2
// \brief Simple structure with one variable of built-in type
struct MyTest2 {
  int c = 41;

  // \brief Default constructor
  //
  // The reconstruction strategy is required for deserialization. A default
  // constructor is one of the reconstruction strategies that checkpoint will
  // look for.
  MyTest2() = default;

  // \brief Templated function for serializing/deserializing
  // a variable of type `MyTest2`
  template <typename Serializer>
  void serialize(Serializer& s) {
    printf("MyTest2 serialize\n");
    s | c;
  }

  // \brief Printing function unto the standard display
  void print() {
    printf("\t MyTest2: c=%d\n", c);
  }
};

// \struct MyTest
//
// \brief Structure with two variables of built-in types and one variable of
// custom type (`MyTest2`)
struct MyTest {
  int a = 29, b = 31;
  MyTest2 my_test_2;

  // \brief Default constructor
  //
  // The reconstruction strategy is required for deserialization. A default
  // constructor is one of the reconstruction strategies that checkpoint will
  // look for.
  MyTest() = default;

  // \brief Printing function unto the standard display
  void print() {
    printf("MyTest: a=%d, b=%d\n", a, b);
    my_test_2.print();
  }

  // \brief Templated function for serializing/deserializing
  // a variable of type `MyTest`
  //
  // \tparam <Serializer> The type of serializer depending on the pass
  // \param[in,out] s the serializer for traversing this class
  //
  // \note The serialize method is typically called three times when
  // (de-)serializing to a byte buffer:
  //
  // 1) Sizing: The first time its called, it sizes all the data it recursively
  // traverses to generate a final size for the buffer.
  //
  // 2) Packing: As the traversal occurs, it copies the data traversed to the
  // byte buffer in the appropriate location.
  //
  // 3) Unpacking: As the byte buffer is traversed, it extracts the bytes from
  // the buffer to recursively reconstruct the types and setup the class members.
  //
  template <typename Serializer>
  void serialize(Serializer& s) {
    printf("MyTest serialize\n");
    s | a;
    s | b;

    // Recursive dispatch to the `MyTest2` object
    s | my_test_2;
  }
};

}}} // end namespace checkpoint::intrusive::examples

int main(int, char**) {
  using namespace magistrate::intrusive::examples;

  // Define a variable of custom type `MyTest`
  MyTest my_test_inst;
  my_test_inst.a = 10;
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

  // De-serialization call to create a new unique pointer to `MyTest`
  auto t = checkpoint::deserialize<MyTest>(ret->getBuffer());
  t->print();

  return 0;
}

/// [Serialize custom structure]
