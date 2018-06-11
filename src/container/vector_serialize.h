
#if !defined INCLUDED_SERDES_VECTOR_SERIALIZE
#define INCLUDED_SERDES_VECTOR_SERIALIZE

#include "serdes_common.h"
#include "serializers/serializers_headers.h"

#include <vector>

namespace serdes {

template <typename Serializer, typename T>
void serializeVectorMeta(Serializer& s, std::vector<T>& vec) {
  SizeType vec_size = vec.size();
  s | vec_size;
  vec.resize(vec_size);
}

template <typename Serializer, typename T>
void serialize(Serializer& s, std::vector<T>& vec) {
  serializeVectorMeta(s, vec);
  serializeArray(s, &vec[0], vec.size());
}

template <typename Serializer, typename T>
void parserdesVectorMeta(Serializer& s, std::vector<T>& vec) {
  SizeType vec_size = vec.size();
  s & vec_size;
  vec.resize(vec_size);
}

template <typename Serializer, typename T>
void parserdes(Serializer& s, std::vector<T>& vec) {
  parserdesVectorMeta(s, vec);
  parserdesArray(s, &vec[0], vec.size());
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_VECTOR_SERIALIZE*/
