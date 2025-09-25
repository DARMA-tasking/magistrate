/*
//@HEADER
// *****************************************************************************
//
//                            container_serialize.h
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

#if !defined INCLUDED_SRC_CHECKPOINT_CONTAINER_CONTAINER_SERIALIZE_H
#define INCLUDED_SRC_CHECKPOINT_CONTAINER_CONTAINER_SERIALIZE_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/serializers_headers.h"

namespace checkpoint {

// Template to determine the clean value type for a container
namespace detail {

template <typename T, typename = void>
struct get_value_type : std::false_type {
  using value_type = typename T::value_type;
};

template <typename T>
struct get_value_type<
  T, std::void_t<typename T::mapped_type>
> : std::true_type {
  using value_type = std::pair<typename T::key_type, typename T::mapped_type>;
};

} /* end detail namespace */

template <typename Serializer, typename ContainerT>
inline typename ContainerT::size_type
serializeContainerSize(Serializer& s, ContainerT& cont) {
  typename ContainerT::size_type cont_size = cont.size();
  if (s.isFootprinting()) {
    s.countBytes(cont);
  } else {
    s | cont_size;
  }
  return cont_size;
}

template <typename Serializer, typename ContainerT>
inline typename ContainerT::size_type
serializeContainerCapacity(Serializer& s, ContainerT& cont) {
  typename ContainerT::size_type cont_capacity = cont.capacity();
  if (!s.isFootprinting()) {
    s | cont_capacity;
  }
  return cont_capacity;
}

template <typename Serializer, typename ContainerT>
inline void serializeContainerElems(Serializer& s, ContainerT& cont) {
  // Without error checking, we can easily iterate and serialize by iterating
  // through. However, since we are modifying the type to eliminate const from
  // std::pair<X const, Y>, error checking identifies the type mismatch. Here,
  // we adjust the types to align with the serialization process based on the
  // resulting value_type. For instance, in the case of std::set, the value_type
  // is always const because the elements cannot be altered, necessitating the
  // use of const_cast to remove the const qualifier.

#if defined(SERIALIZATION_ERROR_CHECKING)
  using ValueT = typename detail::get_value_type<ContainerT>::value_type;
  for (auto& elm : cont) {
    if constexpr (std::is_same<ValueT&, decltype(elm)>::value) {
      // Case where get_value_type<ContainerT>::value_type matches the
      // value_type of the container (no type change is necessary)
      s | elm;
    } else if constexpr (std::is_same<ValueT const&, decltype(elm)>::value) {
      // Case where get_value_type<ContainerT>::value_type just has an added
      // const, this occurs for \c std::set<T>, because the elements can not be
      // modified during iteration, but the deserialization uses the non-const
      // type to reconstruct value_type and put it in the container
      s | const_cast<ValueT&>(elm);
    } else {
      // Case where get_value_type<ContainerT>::value_type has a const removed
      // inside a pair, occurring for std::map, which can't be handled with
      // const_cast. Thus, we reinterpret_cast the const out for error checking.
      s | reinterpret_cast<ValueT&>(elm);
    }
  }
#else
  for (auto& elm : cont) {
    s | elm;
  }
#endif
}

} /* end namespace checkpoint */

#endif /*INCLUDED_SRC_CHECKPOINT_CONTAINER_CONTAINER_SERIALIZE_H*/
