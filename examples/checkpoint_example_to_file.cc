/*
//@HEADER
// *****************************************************************************
//
//                           checkpoint_example_to_file.cc
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

/// [Serialize structure to file]

#include <checkpoint/checkpoint.h>

#include <iostream>
#include <vector>

namespace magistrate::intrusive::examples {

static constexpr int const u_val = 934;

// \struct MyTestType
// \brief Simple structure with two variables of built-in types
struct MyTestType {

  // \brief Default constructor
  //
  // The default constructor is needed for the (de)serialization.
  // (required for serialization)
  MyTestType() = default;

  // \brief Constructor with two parameters
  //
  // \param[in] Initial value for `a`
  // \param[in] Initial value for `b`
  //
  explicit MyTestType(int len) : u_(), len_(len)
  {
    u_.resize(len_);
    for (int i = 0; i < len_; ++i)
      u_[i] = u_val + i;
  }

  // \brief Templated function for serializing/deserializing
  // a variable of type `MyTestType`
  //
  // \tparam <Serializer> { Type for storing the serialized result }
  // \param[in,out] Variable for storing the serialized result
  //
  // \note The routine `serialize` is actually a two-way routine:
  // - it creates the serialized result `s` by combining
  //   the variables `u_` and `len_`; this creation phase is run
  //   when the status of the serializer `s` is `Packing`.
  // - it can extract from a serialized result `s` the values
  //   to place in the variables `u_` and `len_`; this extraction phase is run
  //   when the status of the serializer `s` is `Unpacking`.
  //
  template <typename Serializer>
  void serialize(Serializer& s) {
    s | u_;
    s | len_;
  }

  //
  // Friend functions
  //
  friend bool operator== (const MyTestType &c1, const MyTestType &c2);

  //
  // Variables
  //

  std::vector<double> u_;
  int len_ = 0;

};


bool operator==(const MyTestType &c1, const MyTestType &c2)
{
  if (c1.len_ != c2.len_)
    return false;
  bool isEqual = true;
  for (int i = 0; i < c1.len_; ++i) {
    if (c1.u_[i] != c2.u_[i]) {
      isEqual = false;
      break;
    }
  }
  return isEqual;
}

} // end namespace magistrate::intrusive::examples


int main(int, char**) {
  using namespace magistrate::intrusive::examples;

  // Define a variable of custom type `MyTestType`
  MyTestType my_test_inst(11);

  // Call the serialization routine for the variable `my_test_inst`
  // The output is a unique pointer: `std::unique_ptr<SerializedInfo>`
  // (defined in `src/checkpoint_api.h`)
  checkpoint::serializeToFile(my_test_inst, "hello.txt");

  //
  // De-serializes from the file an object of type 'MyTestType'
  // out will be an object of type 'std::unique_ptr<MyTestType>'
  //
  auto out = checkpoint::deserializeFromFile<MyTestType>("hello.txt");

  if (my_test_inst == *out)
    std::cout << " Serialization / Deserialization from file worked. \n";
  else
    std::cout << " Serialization / Deserialization from file failed. \n";

  //
  // Another option is to de-serialize into an existing object of type 'MyTestType'
  //
  MyTestType out_2;

  //
  // Here 'out_2' will contain an empty vector and an integer 'len_' set to 0.
  //

  checkpoint::deserializeInPlaceFromFile<MyTestType>("hello.txt", &out_2);

  //
  // Now 'out_2' will contain:
  // - a resized vector filled with the values stored in the file;
  // - an integer 'len_' equal to the length of the vector stored in the file.
  //

  if (my_test_inst == out_2)
    std::cout << " Deserialization in-place from file worked. \n";
  else
    std::cout << " Deserialization in-place from file failed. \n";

  return 0;
}


/// [Serialize structure to file]

