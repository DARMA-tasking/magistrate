
#if !defined INCLUDED_SERDES_MEMORY_SERDES
#define INCLUDED_SERDES_MEMORY_SERDES

#include "serdes_common.h"
#include "base_serializer.h"

namespace serdes {

struct MemorySerializer : Serializer {
  MemorySerializer(ModeType const& in_mode, SerialByteType* in_start)
    : Serializer(in_mode), start_(in_start), cur_(in_start)
  { }

  explicit MemorySerializer(ModeType const& in_mode)
    : Serializer(in_mode)
  { }

  SerialByteType* getBuffer() const {
    return start_;
  }

  void initializeBuffer(SerialByteType* const ptr) {
    start_ = cur_ = ptr;
  }

  SerialByteType* getSpotIncrement(SerialSizeType const inc) {
    SerialByteType* spot = cur_;
    cur_ += inc;
    return spot;
  }

protected:
  // The start of the memory buffer to serialize into or out of
  SerialByteType* start_ = nullptr;

  // The current location of the memory buffer for the next call
  SerialByteType* cur_ = nullptr;
};

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_MEMORY_SERDES*/
