
#if !defined INCLUDED_SERDES_PACKER
#define INCLUDED_SERDES_PACKER

#include "serdes_common.h"
#include "memory_serializer.h"
#include "buffer/buffer.h"
#include "buffer/managed_buffer.h"
#include "buffer/user_buffer.h"

namespace serdes {

template <typename BufferT>
struct PackerBuffer : MemorySerializer {
  using BufferTPtrType = std::unique_ptr<BufferT>;
  using PackerReturnType = std::tuple<BufferTPtrType, SerialSizeType>;

  PackerBuffer(SerialSizeType const& in_size);
  PackerBuffer(SerialSizeType const& in_size, BufferTPtrType buf_ptr);

  void contiguousBytes(void* ptr, SerialSizeType size, SerialSizeType num_elms);
  BufferTPtrType extractPackedBuffer();

private:
  // Size of the buffer we are packing (Sizer should have run already)
  SerialSizeType const size_;

  // The abstract buffer that may manage the memory in various ways
  BufferTPtrType buffer_ = nullptr;
};

using Packer = PackerBuffer<ManagedBuffer>;
using PackerUserBuf = PackerBuffer<UserBuffer>;

} /* end namespace serdes */

#include "packer.impl.h"

#endif /*INCLUDED_SERDES_PACKER*/
