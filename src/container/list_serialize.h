
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

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_LIST_SERIALIZE*/
