
#include "serdes_common.h"
#include "memory_serializer.h"
#include "unpacker.h"

#include <cstdlib>
#include <cstring>

namespace serdes {

Unpacker::Unpacker(SerialByteType* buf, SerialSizeType const& size)
  : MemorySerializer(ModeType::Unpacking, buf)
{
  debug_serdes("Unpacker: size=%ld, start_=%p, cur_=%p\n", size, start_, cur_);
}

void Unpacker::contiguousBytes(void* ptr, SerialSizeType size, SerialSizeType num_elms) {
  SerialSizeType const len = size * num_elms;
  SerialByteType* spot = this->getSpotIncrement(len);
  std::memcpy(ptr, spot, len);
}

} /* end namespace serdes */

