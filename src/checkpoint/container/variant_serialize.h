/*
//@HEADER
// *****************************************************************************
//
//                             varaint_serialize.h
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

#if !defined INCLUDED_CHECKPOINT_CONTAINER_VARIANT_SERIALIZE_H
#define INCLUDED_CHECKPOINT_CONTAINER_VARIANT_SERIALIZE_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/serializers_headers.h"
#include "checkpoint/dispatch/allocator.h"

#include <variant>

namespace checkpoint::detail {

template <typename... Args>
struct SerializeEntry;

template <typename Arg, typename... Args>
struct SerializeEntry<Arg, Args...> {

  template <typename SerializerT, typename VariantT>
  static void serialize(
    SerializerT& s, VariantT& v, std::size_t entry, std::size_t cur
  ) {
    if (entry == cur) {
      if (s.isUnpacking()) {
        using Alloc = dispatch::Allocator<Arg>;
        using Reconstructor =
          dispatch::Reconstructor<typename dispatch::CleanType<Arg>::CleanT>;
        Alloc allocated;
        auto* reconstructed = Reconstructor::construct(allocated.buf);
        s | *reconstructed;
        v = std::move(*reconstructed);
      } else {
        s | std::get<Arg>(v);
      }
    } else {
      SerializeEntry<Args...>::serialize(s, v, entry, cur+1);
    }
  }
};

template <>
struct SerializeEntry<> {
  template <typename SerializerT, typename VariantT>
  static void serialize(
    SerializerT& s, VariantT& v, std::size_t entry, std::size_t cur
  ) {
    // base case
  }
};

template <typename... Args>
struct ByteCopyableVariant;

template <typename Arg, typename... Args>
struct ByteCopyableVariant<Arg, Args...> {
  static constexpr bool const byte_copyable =
    SerializableTraits<Arg>::is_bytecopyable and
    ByteCopyableVariant<Args...>::byte_copyable;
};

template <>
struct ByteCopyableVariant<> {
  static constexpr bool const byte_copyable = true;
};

} /* end namespace checkpoint::detail */

namespace checkpoint {

template <typename... Args>
struct ByteCopyNonIntrusive<std::variant<Args...>> {
  using isByteCopyable =
    std::bool_constant<detail::ByteCopyableVariant<Args...>::byte_copyable>;
};

template <typename SerializerT, typename... Args>
void serialize(SerializerT& s, std::variant<Args...>& v) {
  std::size_t entry = v.index();
  s | entry;
  detail::SerializeEntry<Args...>::serialize(s, v, entry, 0);
}

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_CONTAINER_VARIANT_SERIALIZE_H*/
