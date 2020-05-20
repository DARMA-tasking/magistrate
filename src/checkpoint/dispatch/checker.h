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
#include "checkpoint/dispatch/clean_type.h"

#include <stack>
#include <unordered_set>
#include <vector>
#include <tuple>
#include <string>
#include <memory>
#include <cassert>

namespace checkpoint { namespace dispatch {

struct CountRecord {
  CountRecord() = default;

  explicit CountRecord(std::string in_name)
    : name_(in_name)
  { }

  void addSerializedMember(void* addr) { serialized_members.insert(addr); }
  void addMember(void* addr, std::string name) {
    all_members.push_back(std::make_tuple(addr,name));
  }
  void addIgnoredMember(void* addr) {
    ignored_members.insert(addr);
  }

  std::string getName() const { return name_; }

private:
  std::string name_ = 0;
public:
  std::unordered_set<void*> serialized_members;
  std::vector<std::tuple<void*,std::string>> all_members;
  std::unordered_set<void*> ignored_members;
};

template <typename SerializerT, typename T>
struct CounterDispatch {
  static void serializeIntrusive(SerializerT& s, T& t);
  static void serializeNonIntrusiveEnum(SerializerT& s, T& t);
  static void serializeNonIntrusive(SerializerT& s, T& t);
};

/// Custom traverser for checking serialize functions
struct BaseCounter : checkpoint::Serializer {
  template <typename U, typename V>
  using DispatcherType = CounterDispatch<U, V>;

  BaseCounter() : BaseCounter(checkpoint::eSerializationMode::None, false) { }

  BaseCounter(checkpoint::eSerializationMode in_mode, bool in_print_missing)
    : checkpoint::Serializer(in_mode),
      stack_(std::make_shared<std::stack<CountRecord>>()),
      print_missing_(in_print_missing)
  { }

  template <typename CounterLike>
  explicit BaseCounter(CounterLike& cl)
    : checkpoint::Serializer(cl.getMode()),
      stack_(cl.getStack()),
      print_missing_(cl.printMissing())
  { }

  ~BaseCounter() {
    for (auto&& missing : missing_set_) {
      printf("Missing serializer for %s\n", missing.c_str());
    }
  }

  template <typename T>
  void check(T& t, std::string t_name) {
    assert(stack_->size() > 0 && "Must have valid live stack");
    stack_->top().addMember(reinterpret_cast<void*>(cleanType(&t)), t_name);
  }

  template <typename T>
  void ignore(T& t, std::string t_name) {
    assert(stack_->size() > 0 && "Must have valid live stack");
    stack_->top().addIgnoredMember(reinterpret_cast<void*>(cleanType(&t)));
  }

  template <typename SerializerT, typename T>
  void contiguousTyped(SerializerT&, T* t, std::size_t num_elms) {
    assert(stack_->size() > 0 && "Must have valid live stack");
    stack_->top().addSerializedMember(reinterpret_cast<void*>(t));
  }

  void contiguousBytes(void* ptr, std::size_t size, std::size_t num_elms) { }

  std::shared_ptr<std::stack<CountRecord>> getStack() {
    return stack_;
  }

  bool printMissing() const { return print_missing_; }

  void addMissing(std::string const& missing) {
    missing_set_.insert(missing);
  }

  std::unordered_set<std::string> const& getMissing() const {
    return missing_set_;
  }

  template <typename U, typename V>
  friend struct CounterDispatch;

protected:
  std::shared_ptr<std::stack<CountRecord>> stack_;
  std::unordered_set<std::string> missing_set_;
  bool print_missing_ = false;
};

// Type that does special dispatch
struct Counter : BaseCounter { };

// Type that invokes regular serializer overload
struct NonCounter : BaseCounter {
  explicit NonCounter(Counter& c) : BaseCounter(c) { }
};

}} /* end namespace checkpoint::dispatch */

#include "checkpoint/dispatch/checker.impl.h"

#endif /*INCLUDED_CHECKPOINT_DISPATCH_CHECKER_H*/
