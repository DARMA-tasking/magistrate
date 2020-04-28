/*
//@HEADER
// *****************************************************************************
//
//                             checkpoint_file.cc
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

#include <cstdio>
#include <fstream>

namespace serdes { namespace examples {

struct MyTest2 {
  int c = 41;
  std::vector<double> vec;

  MyTest2() = default;

  explicit MyTest2(std::size_t vec_len)
    : vec(vec_len)
  {
    for (std::size_t i = 0; i < vec.size(); i++) {
      vec[i] = i * 29.3;
    }
  }

  template <typename Serializer>
  void serialize(Serializer& s) {
    printf("MyTest2::serialize\n");
    s | c;
    s | vec;
  }

  void print() {
    printf("\t MyTest2::print: c=%d\n", c);
    for (std::size_t i = 0; i < vec.size(); i++) {
      printf("\t vec[%ld]=%f\n", i, vec[i]);
    }
  }
};

struct MyTest {
  int a = 29, b = 31;
  MyTest2 my_test_2;

  MyTest() = default;

  explicit MyTest(std::size_t len)
    : my_test_2(len)
  { }

  void print() {
    printf("MyTest::print a=%d, b=%d\n", a, b);
    my_test_2.print();
  }

  template <typename Serializer>
  void serialize(Serializer& s) {
    printf("MyTest::serialize\n");
    s | a;
    s | b;
    s | my_test_2;
  }
};

}} // end namespace serdes::examples

int main(int argc, char** argv) {
  using namespace serdes::examples;

  std::string name = "";
  if (argc == 2) {
    name = argv[1];
  }

  printf("filename name=%s\n", name.c_str());

  // If file exists, de-serialize from the file
  std::ifstream input(name.c_str(), std::ios::in | std::ios::binary);
  if (input.good()) {
    printf("File %s exists, de-serializing from file\n", name.c_str());

    // First, get the number of bytes in the file by reading to the end and
    // ignoring
    input.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize len = input.gcount();
    input.clear();
    input.seekg(0, std::ios::beg);

    printf("File %s contains %ld bytes\n", name.c_str(), len);

    char* buffer = static_cast<char*>(malloc(len));
    input.read(buffer, len);

    auto t = serdes::deserializeType<MyTest>(buffer, len);
    t->print();

    delete t;

  } else {
    printf("File %s does not exist, serializing into file\n", name.c_str());

    // Construct an instance to serialize and fill with some data
    MyTest my_test(8);
    my_test.a = 10;

    // Print the state of the object
    my_test.print();

    // Serialize the object into bytes
    auto serialized = serdes::serializeType<MyTest>(my_test);
    auto& info = std::get<0>(serialized);

    char* buf = info->getBuffer();
    std::size_t buf_len = info->getSize();

    printf("Serialized ptr=%p, len=%ld\n", static_cast<void*>(buf), buf_len);

    // Write to the file specified
    std::ofstream file;
    file.open(name, std::ios::out | std::ios::binary);
    assert(file.is_open());

    file.write(buf, buf_len);
    file.close();

    printf("Written file\n");
  }


  return 0;
}
