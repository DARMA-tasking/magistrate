
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
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
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
