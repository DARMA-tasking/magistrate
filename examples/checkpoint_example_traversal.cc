/*
//@HEADER
// *****************************************************************************
//
//                           checkpoint_traversal.cc
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

/// [Serialization with custom traverser]

#include <checkpoint/checkpoint.h>

#include <cstdio>
#include <string>

namespace magistrate { namespace intrusive { namespace examples {

struct TestObject {

  TestObject() = default;

  struct MakeTag { };

  explicit TestObject(MakeTag) {
    for (int i = 0; i < 10; i++) {
      vec1.push_back(i);
      vec2.push_back(i * 29.34);
      vec3.push_back("hello: " + std::to_string(vec2.back()));
    }
  }

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a | b | vec1 | vec2 | vec3;
  }

private:
  int a = 29;
  int b = 36;
  std::vector<int> vec1;
  std::vector<double> vec2;
  std::vector<std::string> vec3;
};

}}} // end namespace magistrate::intrusive::examples

/// Custom traverser for printing raw bytes
struct PrintBytesTraverse : checkpoint::Serializer {
  PrintBytesTraverse() : checkpoint::Serializer(checkpoint::eSerializationMode::None) { }

  void contiguousBytes(void* ptr, std::size_t size, std::size_t num_elms) {
    printf("PrintBytesTraverse: size=%zu, num_elms=%zu\n", size, num_elms);
  }
};

template <typename U>
struct ToString {
  static std::string apply(U& u) { return std::to_string(u); }
};

template <>
struct ToString<std::string> {
  static std::string apply(std::string& u) { return u; }
};

/// Custom dispatcher to customizing type-specific traversal behavior
/// For non-vectors, dispatch normally. For vectors, print the data inside
template <typename SerializerT, typename T>
struct CustomDispatch {
  static void serializeIntrusive(SerializerT& s, T& t) {
    t.serialize(s);
  }
  static void serializeNonIntrusive(SerializerT& s, T& t) {
    serialize(s, t);
  }
};

// std::vector specialization for dispatcher, vector is always non-intrusive so
// skip that overload
template <typename SerializerT, typename U>
struct CustomDispatch<SerializerT, std::vector<U>> {
  static void serializeNonIntrusive(SerializerT& s, std::vector<U>& t) {
    // Do something special here: e.g., an RDMA for the vector during packing
    printf("Traversing vector: size=%zu\n", t.size());
    for (std::size_t i = 0; i < t.size(); i++) {
      printf("\t vector[%zu]=%s", i, ToString<U>::apply(t[i]).c_str());
    }
    printf("\n");
  }
};

/// Custom traverser for printing typed ranges
struct TypedTraverse : checkpoint::Serializer {
  template <typename U, typename V>
  using DispatcherType = CustomDispatch<U, V>;

  TypedTraverse() : checkpoint::Serializer(checkpoint::eSerializationMode::None) { }

  template <typename SerializerT, typename T>
  void contiguousTyped(SerializerT&, T*, std::size_t num_elms) {
    printf("TypedTraverse: type is %s, num=%zu\n", typeid(T).name(), num_elms);
  }
};

int main(int, char**) {
  using namespace magistrate::intrusive::examples;

  TestObject my_obj(TestObject::MakeTag{});

  // Traverse my_obj with a custom traverser that prints the bytes
  checkpoint::dispatch::Traverse::with<TestObject, PrintBytesTraverse>(my_obj);

  // Traverse my_obj with a custom traverser and dispatcher that prints the
  // types and lens
  checkpoint::dispatch::Traverse::with<TestObject, TypedTraverse>(my_obj);

  return 0;
}

/// [Serialization with custom traverser]
