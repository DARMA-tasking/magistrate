
#if !defined INCLUDED_SERDES_TUPLE_SERIALIZE
#define INCLUDED_SERDES_TUPLE_SERIALIZE

#include "serdes_common.h"
#include "serdes.h"

#include <tuple>
#include <utility>

namespace serdes {

template <typename Serializer, typename... T, size_t... Idxs>
void tuple_helper(Serializer& s, std::tuple<T...>& tup, std::index_sequence<Idxs...>) {
  std::forward_as_tuple(
    ((s | std::get<Idxs>(tup)),0)...
  );
}

template <typename Serializer, typename... Args>
void serialize(Serializer& s, std::tuple<Args...>& tuple) {
  tuple_helper(s, tuple, std::index_sequence_for<Args...>());
}

template <typename Serializer, typename T, typename U>
void serialize(Serializer& s, std::pair<T, U>& pair) {
  s | pair.first;
  s | pair.second;
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_TUPLE_SERIALIZE*/
