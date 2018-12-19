
#if !defined INCLUDED_SERDES_VECTOR_SERIALIZE
#define INCLUDED_SERDES_VECTOR_SERIALIZE

#include "serdes_common.h"
#include "serializers/serializers_headers.h"

#include <vector>

namespace serdes {

template <typename Serializer, typename T, typename VectorAllocator>
void serializeVectorMeta(Serializer& s, std::vector<T, VectorAllocator>& vec) {
  SizeType vec_size = vec.size();
  s | vec_size;
  vec.resize(vec_size);
}

template <typename Serializer, typename T, typename VectorAllocator>
void serialize(Serializer& s, std::vector<T, VectorAllocator>& vec) {
  serializeVectorMeta(s, vec);
  serializeArray(s, &vec[0], vec.size());
}

template <typename Serializer, typename T, typename VectorAllocator>
void parserdesVectorMeta(Serializer& s, std::vector<T, VectorAllocator>& vec) {
  SizeType vec_size = vec.size();
  s & vec_size;
  vec.resize(vec_size);
}

template <typename Serializer, typename T, typename VectorAllocator>
void parserdes(Serializer& s, std::vector<T, VectorAllocator>& vec) {
  parserdesVectorMeta(s, vec);
  parserdesArray(s, &vec[0], vec.size());
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_VECTOR_SERIALIZE*/
