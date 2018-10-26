
#if !defined INCLUDED_SERDES_ARRAY_SERIALIZE
#define INCLUDED_SERDES_ARRAY_SERIALIZE

#include "serdes_common.h"
#include "serializers/serializers_headers.h"

#include <array>

namespace serdes {

template <typename Serializer, typename T, size_t N>
void serialize(Serializer& s, std::array<T, N>& array) {
  serializeArray(s, &array[0], array.size());
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_ARRAY_SERIALIZE*/
