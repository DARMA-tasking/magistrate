
#if !defined INCLUDED_SERDES_UNPACKER
#define INCLUDED_SERDES_UNPACKER

#include "serdes_common.h"
#include "memory_serializer.h"

namespace serdes {

struct Unpacker : MemorySerializer {
  Unpacker(SerialByteType* buf, SizeType const& size);

  void contiguousBytes(void* ptr, SizeType size, SizeType num_elms);
};

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_UNPACKER*/
