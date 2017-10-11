
#if !defined INCLUDED_SERDES_PACKER
#define INCLUDED_SERDES_PACKER

#include "serdes_common.h"
#include "serdes.h"
#include "memory_serializer.h"
#include "managed_buffer.h"

namespace serdes {

template <typename BufferT>
struct PackerBuffer : MemorySerializer {
  using BufferPtrType = std::unique_ptr<BufferT>;

  PackerBuffer(SizeType const& in_size);

  void contiguousBytes(void* ptr, SizeType size, SizeType num_elms);
  BufferPtrType extractPackedBuffer();

private:
  // Size of the buffer we are packing (Sizer should have run already)
  SizeType const size_;

  // The abstract buffer that may manage the memory in various ways
  BufferPtrType buffer_ = nullptr;
};

using Packer = PackerBuffer<ManagedBuffer>;

} /* end namespace serdes */

#include "packer.impl.h"

#endif /*INCLUDED_SERDES_PACKER*/
