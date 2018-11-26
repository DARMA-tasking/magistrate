/*
//@HEADER
// ************************************************************************
//
//                          dispatch_deserializer.h
//                           checkpoint
//              Copyright (C) 2017 NTESS, LLC
//
// Under the terms of Contract DE-NA-0003525 with NTESS, LLC,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// ************************************************************************
//@HEADER
*/

#if ! defined INCLUDED_SERDES_DISPATCH_DESERIALIZER
#define INCLUDED_SERDES_DISPATCH_DESERIALIZER

#include "serdes_common.h"
#include "serdes_all.h"
#include "traits/serializable_traits.h"

#include <type_traits>
#include <tuple>
#include <cstdlib>

namespace serdes {

template <typename SerializerT, typename T>
struct DeserializerDispatch {
  template <typename U>
  using isDefaultConsType =
    typename std::enable_if<std::is_default_constructible<U>::value, T>::type;

  template <typename U>
  using isNotDefaultConsType =
    typename std::enable_if<not std::is_default_constructible<U>::value, T>::type;

  // If we have the detection component, we can more precisely check for
  // reconstuctibility
  #if HAS_DETECTION_COMPONENT
    template <typename U>
    using isReconstructibleType =
      typename std::enable_if<SerializableTraits<U>::is_reconstructible, T>::type;

    template <typename U>
    using isNonIntReconstructibleType =
      typename std::enable_if<
        SerializableTraits<U>::is_nonintrusive_reconstructible, T
      >::type;
  #else
    template <typename U>
    using isNonIntReconstructibleType = isNotDefaultConsType<U>;
  #endif

  template <typename U = T>
  T& operator()(
    SerializerT& s, void* buf,
    isDefaultConsType<U>* __attribute__((unused)) x = nullptr
  ) {
    debug_serdes("DeserializerDispatch: default constructor: buf=%p\n", buf);
    T* t_ptr = new (buf) T{};
    auto& t = *t_ptr;
    return t;
  }

  #if HAS_DETECTION_COMPONENT
  template <typename U = T>
  T& operator()(
    SerializerT& s, void* buf,
    isReconstructibleType<U>*  __attribute__((unused)) x = nullptr
  ) {
    debug_serdes("DeserializerDispatch: T::reconstruct(): buf=%p\n", buf);
    return T::reconstruct(buf);
  }
  #endif

  template <typename U = T>
  T& operator()(
    SerializerT& s, void* buf,
    isNonIntReconstructibleType<U>*  __attribute__((unused)) x = nullptr
  ) {
    debug_serdes("DeserializerDispatch: non-int reconstruct(): buf=%p\n", buf);
    T* t = nullptr;
    reconstruct(s,t,buf);
    return *t;
  }
};

} //end namespace serdes

#endif /*INCLUDED_SERDES_DISPATCH_DESERIALIZER*/
