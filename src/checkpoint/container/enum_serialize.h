
#if !defined INCLUDED_SERDES_ENUM_SERIALIZE
#define INCLUDED_SERDES_ENUM_SERIALIZE

#include "checkpoint/serdes_common.h"
#include "checkpoint/serializers/serializers_headers.h"

#include <type_traits>

namespace serdes {

template <
  typename Serializer,
  typename Enum,
  typename Underlying = std::underlying_type_t<Enum>
>
void serializeEnum(Serializer& s, Enum& e) {
    Underlying u = static_cast<Underlying>(e);
    s | u;
    e = static_cast<Enum>(u);
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_ENUM_SERIALIZE*/
