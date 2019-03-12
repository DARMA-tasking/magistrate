
#include "serdes_common.h"
#include "memory_serializer.h"
#include "buffer/buffer.h"

#include <cstring>

namespace serdes {

template <typename BufferT>
PackerBuffer<BufferT>::PackerBuffer(SerialSizeType const& in_size)
   : MemorySerializer(ModeType::Packing), size_(in_size),
     buffer_(std::make_unique<BufferT>(size_))
{
  MemorySerializer::initializeBuffer(buffer_->getBuffer());
  debug_serdes(
    "PackerBuffer: size=%ld, start_=%p, cur_=%p\n", size_, start_, cur_
  );
}

template <typename BufferT>
PackerBuffer<BufferT>::PackerBuffer(
  SerialSizeType const& in_size, BufferTPtrType buf_ptr
) : MemorySerializer(ModeType::Packing), size_(in_size),
    buffer_(std::move(buf_ptr))
{
  MemorySerializer::initializeBuffer(buffer_->getBuffer());
  debug_serdes(
    "PackerBuffer: size=%ld, start_=%p, cur_=%p\n", size_, start_, cur_
  );
}

template <typename BufferT>
typename PackerBuffer<BufferT>::BufferTPtrType
PackerBuffer<BufferT>::extractPackedBuffer() {
  auto ret = std::move(buffer_);
  buffer_ = nullptr;
  return ret;
}

template <typename BufferT>
void PackerBuffer<BufferT>::contiguousBytes(
  void* ptr, SerialSizeType size, SerialSizeType num_elms
) {
  debug_serdes(
    "PackerBuffer: offset=%ld, size=%ld, num_elms=%ld, ptr=%p, cur_=%p, val=%d\n",
    cur_ - start_, size_, num_elms, ptr, cur_, *reinterpret_cast<int*>(ptr)
  );

  SerialSizeType const len = size * num_elms;
  SerialByteType* spot = this->getSpotIncrement(len);
  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
  std::memcpy(spot, ptr, len);
}

} /* end namespace serdes */
