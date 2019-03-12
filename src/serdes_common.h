
#if !defined INCLUDED_SERDES_COMMON
#define INCLUDED_SERDES_COMMON

#define DEBUG_SERDES 0

#if DEBUG_SERDES
#define debug_serdes(debug_str, ...)            \
  do {                                          \
    printf(debug_str, __VA_ARGS__);             \
  } while (0);
#include <cstdio>
#else
#define debug_serdes(debug_str, ...)
#endif

#include <cstdlib>
#include <cstdint>
#include <functional>

namespace serdes {

using SerialSizeType = size_t;
using SerialByteType = char;

using BufferObtainFnType = std::function<SerialByteType*(SerialSizeType size)>;

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_COMMON*/
