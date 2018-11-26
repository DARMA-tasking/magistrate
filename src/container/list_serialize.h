/*
//@HEADER
// ************************************************************************
//
//                          list_serialize.h
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

#if !defined INCLUDED_SERDES_LIST_SERIALIZE
#define INCLUDED_SERDES_LIST_SERIALIZE

#include "serdes_common.h"
#include "serializers/serializers_headers.h"
#include "container_serialize.h"

#include <list>
#include <deque>

namespace serdes {

template <typename Serializer, typename ContainerT, typename ElmT>
inline void deserializeOrderedElems(
  Serializer& s, ContainerT& cont, typename ContainerT::size_type size
) {
  for (auto i = 0; i < size; i++) {
    ElmT elm;
    s | elm;
    cont.push_back(elm);
  }
}

template <typename Serializer, typename ContainerT>
inline void serializeOrderedContainer(Serializer& s, ContainerT& cont) {
  using ValueT = typename ContainerT::value_type;

  typename ContainerT::size_type size = serializeContainerSize(s, cont);

  if (s.isUnpacking()) {
    deserializeOrderedElems<Serializer, ContainerT, ValueT>(s, cont, size);
  } else {
    serializeContainerElems<Serializer, ContainerT>(s, cont);
  }
}

template <typename Serializer, typename T>
inline void serialize(Serializer& s, std::list<T>& lst) {
  serializeOrderedContainer(s, lst);
}

template <typename Serializer, typename T>
inline void serialize(Serializer& s, std::deque<T>& lst) {
  serializeOrderedContainer(s, lst);
}

/* parserdes */
template <typename Serializer, typename ContainerT, typename ElmT>
inline void deparserdesOrderedElems(
  Serializer& s, ContainerT& cont, typename ContainerT::size_type size
) {
  for (auto i = 0; i < size; i++) {
    ElmT elm;
    s & elm;
    cont.push_back(elm);
  }
}

template <typename Serializer, typename ContainerT>
inline void parserdesOrderedContainer(Serializer& s, ContainerT& cont) {
  using ValueT = typename ContainerT::value_type;

  typename ContainerT::size_type size = parserdesContainerSize(s, cont);

  if (s.isUnpacking()) {
    deparserdesOrderedElems<Serializer, ContainerT, ValueT>(s, cont, size);
  } else {
    parserdesContainerElems<Serializer, ContainerT>(s, cont);
  }
}

template <typename Serializer, typename T>
inline void parserdes(Serializer& s, std::list<T>& lst) {
  parserdesOrderedContainer(s, lst);
}

template <typename Serializer, typename T>
inline void parserdes(Serializer& s, std::deque<T>& lst) {
  parserdesOrderedContainer(s, lst);
}


} /* end namespace serdes */

#endif /*INCLUDED_SERDES_LIST_SERIALIZE*/
