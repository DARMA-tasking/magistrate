/*
//@HEADER
// *****************************************************************************
//
//                                 clean_type.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_CLEAN_TYPE_H
#define INCLUDED_CHECKPOINT_DISPATCH_CLEAN_TYPE_H

#include "checkpoint/common.h"

namespace checkpoint { namespace dispatch {

template <typename T>
struct CleanType {

  template <typename U>
  using isConst = typename std::enable_if<std::is_const<U>::value, T>::type;
  template <typename U>
  using isNotConst = typename std::enable_if<!std::is_const<U>::value, T>::type;

  using NonConstT = typename std::remove_const<T>::type;
  using NonConstRefT = typename std::decay<T>::type;
  using CleanT = NonConstRefT;

  template <typename U = T>
  static NonConstRefT* clean(T* val) {
    return CleanType<T>::apply1(val);
  }

  template <typename U = T>
  static NonConstRefT* apply1(T* val, isConst<U>* = nullptr) {
    return reinterpret_cast<NonConstRefT*>(const_cast<NonConstT*>(val));
  }

  template <typename U = T>
  static NonConstRefT* apply1(T* val, isNotConst<U>* = nullptr) {
    return reinterpret_cast<NonConstRefT*>(val);
  }
};

template <typename T>
typename CleanType<T>::CleanT* cleanType(T* val) {
  return CleanType<T>::clean(val);
}

}} /* end namespace checkpoint::dispatch */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_CLEAN_TYPE_H*/
