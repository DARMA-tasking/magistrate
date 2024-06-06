/*
//@HEADER
// *****************************************************************************
//
//                                  detector.h
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

#if !defined INCLUDED_CHECKPOINT_DETECTOR_H
#define INCLUDED_CHECKPOINT_DETECTOR_H

#include <type_traits>

namespace detection {

/*
 * Implementation of the standard C++ detector idiom, used to statically
 * detect/assert properties of C++ types.
 */
struct NoneSuch final {
  NoneSuch() = delete;
  ~NoneSuch() = delete;
  NoneSuch(NoneSuch const&) = delete;
  void operator=(NoneSuch const&) = delete;
};

using NoneSuchType = NoneSuch;

template <typename T, typename, template <typename...> class Op, typename... Args>
struct detector {
  constexpr static auto value = false;
  using value_t = std::false_type;
  using type = T;
  using element_type = T;
};

template <typename T, template <typename...> class Op, typename... Args>
struct detector<T, std::void_t<Op<Args...>>, Op, Args...>  {
  constexpr static auto value = true;
  using value_t = std::true_type;
  using type = Op<Args...>;
  using element_type = typename std::tuple_element<0, std::tuple<Args...>>::type;
};

template <template <typename...> class Op, typename... Args>
using is_detected = detector<NoneSuchType, void, Op, Args...>;

template <template <typename...> class Op, typename... Args>
using detected_t = typename is_detected<Op, Args...>::type;

template <typename ExpectedT, template<typename...> class Op, typename... Args>
using is_detected_exact = std::is_same<detected_t<Op, Args...>, ExpectedT>;

template <typename T, template <typename...> class Op, typename... Args>
using is_detected_convertible = std::is_convertible<detected_t<Op, Args...>, T>;

}  // end detection

#endif /*INCLUDED_CHECKPOINT_DETECTOR_H*/
