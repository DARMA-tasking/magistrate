/*
//@HEADER
// *****************************************************************************
//
//                                  checker.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_CHECKER_H
#define INCLUDED_CHECKPOINT_DISPATCH_CHECKER_H

#include "checkpoint/serializers/base_serializer.h"

#include <stack>
#include <unordered_set>
#include <vector>
#include <tuple>

namespace checkpoint { namespace dispatch {

struct CountRecord {
  CountRecord() = default;

  explicit CountRecord(std::string in_name)
    : name_(in_name)
  { }

  void add(void* addr) { exists.insert(addr); }
  void addCheck(void* addr, std::string name) {
    checks.push_back(std::make_tuple(addr,name));
  }
  void addSkip(void* addr) {
    skips.insert(addr);
  }

  std::string getName() const { return name_; }

private:
  std::string name_ = 0;
public:
  std::unordered_set<void*> exists;
  std::vector<std::tuple<void*,std::string>> checks;
  std::unordered_set<void*> skips;
};

extern std::stack<CountRecord> stack;

template <typename SerializerT, typename T>
struct CounterDispatch {
  static void serializeIntrusive(SerializerT& s, T& t) {
    if (stack.size() > 0) {
      stack.top().add(reinterpret_cast<void*>(&t));
    }

    stack.push(CountRecord{typeid(T).name()});
    t.serialize(s);

    for (auto&& check : stack.top().checks) {
      auto addr = std::get<0>(check);
      auto name = std::get<1>(check);
      auto skip_iter = stack.top().skips.find(addr);
      if (skip_iter == stack.top().skips.end()) {
        auto exists_iter = stack.top().exists.find(addr);
        if (exists_iter == stack.top().exists.end()) {
          printf("Missing serializer for %s\n", name.c_str());
        }
      }
    }

    stack.pop();
  }

  static void serializeNonIntrusiveEnum(SerializerT& s, T& t) {
    if (stack.size() > 0) {
      stack.top().add(reinterpret_cast<void*>(&t));
    }
  }

  static void serializeNonIntrusive(SerializerT& s, T& t) {
    if (stack.size() > 0) {
      stack.top().add(reinterpret_cast<void*>(&t));
    }
    stack.push(CountRecord{typeid(T).name()});
    //printf("\t (%zu) \t NI: starting: %s\n", st.size(), typeid(T).name());
    serialize(s, t);
    //printf("\t (%zu) \t NI: finishing: %s\n", st.size(), typeid(T).name());
    stack.pop();
  }
};

/// Custom traverser for printing raw bytes
struct Counter : checkpoint::Serializer {
  template <typename U, typename V>
  using DispatcherType = CounterDispatch<U, V>;

  Counter() : checkpoint::Serializer(checkpoint::eSerializationMode::None) { }

  template <typename T>
  void check(T& t, std::string t_name) {
    if (stack.size() > 0) {
      stack.top().addCheck(reinterpret_cast<void*>(&t), t_name);
      printf("Adding check for: %s\n", t_name.c_str());
    }
  }

  template <typename T>
  void skip(T& t, std::string t_name) {
    if (stack.size() > 0) {
      stack.top().addSkip(reinterpret_cast<void*>(&t));
    }
  }

  template <typename SerializerT, typename T>
  void contiguousTyped(SerializerT&, T* t, std::size_t num_elms) {
    if (stack.size() > 0) {
      stack.top().add(reinterpret_cast<void*>(t));
    }
    //printf("Counter: type is %s, num=%zu\n", typeid(T).name(), num_elms);
  }

  void contiguousBytes(void* ptr, std::size_t size, std::size_t num_elms) { }
};

}} /* end namespace checkpoint::dispatch */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_CHECKER_H*/
