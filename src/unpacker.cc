
#include "serdes_common.h"
#include "serdes.h"
#include "unpacker.h"

#include <cstdlib>
#include <cstring>

namespace serdes {

Unpacker::Unpacker(SerialByteType* buf, SizeType const& size)
  : MemorySerializer(ModeType::Unpacking, buf)
{
  debug_serdes("Unpacker: size=%ld, start_=%p, cur_=%p\n", size, start_, cur_);
}

void Unpacker::contiguousBytes(void* ptr, SizeType size, SizeType num_elms) {
  SizeType const len = size * num_elms;
  SerialByteType* spot = this->getSpotIncrement(len);
  std::memcpy(ptr, spot, len);
}

} /* end namespace serdes */

