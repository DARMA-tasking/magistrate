
#if !defined INCLUDED_SERDES_STRING_SERIALIZE
#define INCLUDED_SERDES_STRING_SERIALIZE

#include "serdes_common.h"
#include "serializers/serializers_headers.h"

#include <string>

namespace serdes {

template <typename Serializer>
void serializeStringMeta(Serializer& s, std::string& str) {
  SizeType str_size = str.size();
  s | str_size;
  str.resize(str_size);
}

template <typename Serializer>
void serialize(Serializer& s, std::string& str) {
  serializeStringMeta(s, str);
  serializeArray(s, str.c_str(), str.size());
}

template <typename Serializer>
void parserdesStringMeta(Serializer& s, std::string& str) {
  SizeType str_size = str.size();
  s & str_size;
  str.resize(str_size);
}

template <typename Serializer>
void parserdes(Serializer& s, std::string& str) {
  parserdesStringMeta(s, str);
  parserdesArray(s, str.c_str(), str.size());
}

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_STRING_SERIALIZE*/
