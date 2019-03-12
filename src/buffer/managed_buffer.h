
#if !defined INCLUDED_SERDES_MANAGED_BUFFER
#define INCLUDED_SERDES_MANAGED_BUFFER

#include "serdes_common.h"
#include "buffer.h"

#include <memory>

namespace serdes {

struct ManagedBuffer : Buffer {
  using ByteType = SerialByteType[];

  ManagedBuffer(SerialSizeType const& size)
    : size_(size), buffer_(std::make_unique<ByteType>(size))
  { }

  virtual SerialByteType* getBuffer() const override {
    return buffer_.get();
  }

  virtual SerialSizeType getSize() const override {
    return size_;
  }

private:
  SerialSizeType size_ = 0;

  std::unique_ptr<ByteType> buffer_ = nullptr;
};

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_MANAGED_BUFFER*/
