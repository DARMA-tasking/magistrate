
#if !defined INCLUDED_SERDES_MAP_SERIALIZE
#define INCLUDED_SERDES_MAP_SERIALIZE

#include "serdes_common.h"
#include "serializers/serializers_headers.h"
#include "container_serialize.h"

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

namespace serdes {

template <typename Serializer, typename ContainerT, typename ElmT>
inline void deserializeEmplaceElems(
  Serializer& s, ContainerT& cont, typename ContainerT::size_type size
) {
  for (auto i = 0; i < size; i++) {
    ElmT elm;
    s | elm;
    cont.emplace(elm);
  }
}

template <typename Serializer, typename ContainerT>
inline void serializeMapLikeContainer(Serializer& s, ContainerT& cont) {
  using ValueT = typename ContainerT::value_type;

  typename ContainerT::size_type size = serializeContainerSize(s, cont);

  if (s.isUnpacking()) {
    deserializeEmplaceElems<Serializer, ContainerT, ValueT>(s, cont, size);
  } else {
    serializeContainerElems<Serializer, ContainerT>(s, cont);
  }
}

template <typename Serializer, typename T, typename U, typename Comp>
inline void serialize(Serializer& s, std::map<T, U, Comp>& map) {
  serializeMapLikeContainer(s, map);
}

template <typename Serializer, typename T, typename U, typename Comp>
inline void serialize(Serializer& s, std::multimap<T, U, Comp>& map) {
  serializeMapLikeContainer(s, map);
}

template <typename Serializer, typename T, typename Comp>
inline void serialize(Serializer& s, std::set<T, Comp>& set) {
  serializeMapLikeContainer(s, set);
}

template <typename Serializer, typename T, typename Comp>
inline void serialize(Serializer& s, std::multiset<T, Comp>& set) {
  serializeMapLikeContainer(s, set);
}

template <typename Serializer, typename T, typename U, typename Hash, typename Eq>
inline void serialize(Serializer& s, std::unordered_map<T, U, Hash, Eq>& map) {
  serializeMapLikeContainer(s, map);
}

template <typename Serializer, typename T, typename U, typename Hash, typename Eq>
inline void serialize(Serializer& s, std::unordered_multimap<T, U, Hash, Eq>& map) {
  serializeMapLikeContainer(s, map);
}

template <typename Serializer, typename T, typename Hash, typename Eq>
inline void serialize(Serializer& s, std::unordered_set<T, Hash, Eq>& set) {
  serializeMapLikeContainer(s, set);
}

template <typename Serializer, typename T, typename Hash, typename Eq>
inline void serialize(Serializer& s, std::unordered_multiset<T, Hash, Eq>& set) {
  serializeMapLikeContainer(s, set);
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_MAP_SERIALIZE*/
