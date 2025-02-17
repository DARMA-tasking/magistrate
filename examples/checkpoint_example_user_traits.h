/*
//@HEADER
// *****************************************************************************
//
//                       checkpoint_example_user_traits.h
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

#if !defined INCLUDED_EXAMPLES_CHECKPOINT_EXAMPLE_USER_TRAITS_H
#define INCLUDED_EXAMPLES_CHECKPOINT_EXAMPLE_USER_TRAITS_H

#include <checkpoint/checkpoint.h>

struct checkpoint_trait {} CheckpointTrait;
struct shallow_trait {} ShallowTrait;

using checkpoint::has_user_traits_v;
using checkpoint::has_any_user_traits_v;

namespace test {
  struct random_trait {} RandomTrait;

  struct TestObj {
    int a = 1;

    TestObj() {}

    template<
      typename SerT,
      typename std::enable_if_t<
        not has_user_traits_v<SerT, shallow_trait>, void*
      > = nullptr
    >
    void serialize(SerT& s){
      if constexpr(has_user_traits_v<SerT, checkpoint_trait>){
        if(s.isSizing()) printf("Customizing serialization for checkpoint\n");
        s | a;
      } else {
        if(s.isSizing()) printf("Default serializing testObj\n");
      }

      static_assert(not has_user_traits_v<SerT, shallow_trait>,
          "ShallowTrait should have been removed!\n");
    }
  };
}

namespace test {
  template<
    typename SerT,
    typename std::enable_if_t<
      has_user_traits_v<SerT, random_trait>, void*
    > = nullptr
  >
  void serialize(SerT& s, TestObj& myObj){
    if(s.isSizing()){
      printf("Inserting random extra object serialization step! ");
    }
    myObj.serialize(s);
  }

  template<
    typename SerT,
    typename std::enable_if_t<
      has_user_traits_v<SerT, shallow_trait>, void*
    > = nullptr
  >
  void serialize(SerT& s, TestObj& myObj){
    if(s.isSizing()) printf("Removing shallow trait before passing along!\n");
    auto newS = s.template withoutTraits<shallow_trait>();
    myObj.serialize(newS);
  }
}

namespace misc {
  template<
    typename SerT,
    typename std::enable_if_t<
      has_user_traits_v<SerT, test::random_trait>, void*
    > = nullptr
  >
  void serialize(SerT& s, test::TestObj& myObj){
    if(s.isSizing()){
      printf("Serializers in other namespaces don't usually get found ");
    }
    myObj.serialize(s);
  }


  const struct namespace_trait {} NamespaceTrait;
  template<
    typename SerT,
    typename std::enable_if_t<
      has_user_traits_v<SerT, namespace_trait>, void*
    > = nullptr
  >
  void serialize(SerT& s, test::TestObj& myObj){
    if(s.isSizing()){
      printf("A misc:: trait means we can serialize from misc:: too: ");
    }
    myObj.serialize(s);
  }


  const struct hook_all_trait {} HookAllTrait;
  template<
    typename SerT,
    typename T,
    typename std::enable_if_t<
      has_user_traits_v<SerT, hook_all_trait>, void*
    > = nullptr
  >
  void serialize(SerT& s, T& myObj){
    if(s.isSizing()){
      printf("We can even add on a generic pre-serialize hook: ");
    }
    auto newS = s.template withoutTraits<hook_all_trait>();
    myObj.serialize(newS);
  }
}

#endif /*INCLUDED_EXAMPLES_CHECKPOINT_EXAMPLE_USER_TRAITS_H*/
