
#if !defined INCLUDED_SERDES_COMMON
#define INCLUDED_SERDES_COMMON

#define DEBUG_SERDES 0

#if DEBUG_SERDES
#define debug_serdes(debug_str, args...)        \
  do {                                          \
    printf(debug_str, args);                    \
  } while (0);
#include <cstdio>
#else
#define debug_serdes(debug_str, args...)
#endif

#include <cstdlib>
#include <cstdint>
#include <functional>

namespace serdes {

using SizeType = size_t;
using SerialByteType = char;

using BufferObtainFnType = std::function<SerialByteType*(SizeType size)>;

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_COMMON*/
