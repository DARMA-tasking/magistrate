
#if !defined INCLUDED_SERDES_CONTAINER_SERIALIZE
#define INCLUDED_SERDES_CONTAINER_SERIALIZE

#include "serdes_common.h"
#include "serializers/serializers_headers.h"

namespace serdes {

template <typename Serializer, typename ContainerT>
inline typename ContainerT::size_type
serializeContainerSize(Serializer& s, ContainerT& cont) {
  typename ContainerT::size_type cont_size = cont.size();
  s | cont_size;
  return cont_size;
}

template <typename Serializer, typename ContainerT>
inline void serializeContainerElems(Serializer& s, ContainerT& cont) {
  for (auto&& elm : cont) {
    s | elm;
  }
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_CONTAINER_SERIALIZE*/
