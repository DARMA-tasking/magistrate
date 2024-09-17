/*
//@HEADER
// *****************************************************************************
//
//                            view_traits_extract.h
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

#if !defined INCLUDED_SRC_CHECKPOINT_CONTAINER_VIEW_TRAITS_EXTRACT_H
#define INCLUDED_SRC_CHECKPOINT_CONTAINER_VIEW_TRAITS_EXTRACT_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/serializers_headers.h"

#if MAGISTRATE_KOKKOS_ENABLED

#include <Kokkos_Core.hpp>
#include <Kokkos_DynamicView.hpp>
#include <Kokkos_DynRankView.hpp>

#include <utility>
#include <tuple>
#include <type_traits>

namespace checkpoint {

/*
 * Serialization helper classes to count the number of runtime and static
 * dimensions of a Kokkos::View
 */

// This shouldn't be necessary but I can't find the correct trait to extract in
// Kokkos::View, so I will manually extract the underlying data type
template <typename ViewType>
struct ViewGetType;

template <typename T, typename... Args>
struct ViewGetType<Kokkos::View<T,Args...>> {
  using DataType = T;
};

template <typename T, typename... Args>
struct ViewGetType<Kokkos::Experimental::DynamicView<T,Args...>> {
  using DataType = T;
};

template <typename T, typename... Args>
struct ViewGetType<Kokkos::DynRankView<T,Args...>> {
  using DataType = T;
};

template <
  typename ViewType,
  typename T = typename ViewGetType<ViewType>::DataType
>
struct CountDims {
  using BaseT = typename std::decay<T>::type;
  static constexpr size_t dynamic = 0;
  static int numDims(ViewType const&) { return 0; }
};

template <typename ViewType, typename T>
struct CountDims<ViewType, T*> {
  using BaseT = typename CountDims<ViewType,T>::BaseT;
  static constexpr size_t dynamic = CountDims<ViewType, T>::dynamic + 1;

  static int numDims(ViewType const& view) {
    auto const val = CountDims<ViewType, T>::numDims(view);
    return val + 1;
  }
};

template <typename ViewType, typename T, size_t N>
struct CountDims<ViewType, T[N]> {
  using BaseT = typename CountDims<ViewType,T>::BaseT;
  static constexpr size_t dynamic = CountDims<ViewType, T>::dynamic;
  static int numDims(ViewType const& view) {
    auto const val = CountDims<ViewType, T>::numDims(view);
    return val + 1;
  }
};

} /* end namespace checkpoint */

#endif /*MAGISTRATE_KOKKOS_ENABLED*/

#endif /*INCLUDED_SRC_CHECKPOINT_CONTAINER_VIEW_TRAITS_EXTRACT_H*/
