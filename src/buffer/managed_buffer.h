
#if !defined INCLUDED_SERDES_MANAGED_BUFFER
#define INCLUDED_SERDES_MANAGED_BUFFER

#include "serdes_common.h"
#include "buffer.h"

#include <memory>

namespace serdes {

struct ManagedBuffer : Buffer {
  using ByteType = SerialByteType[];

  ManagedBuffer(SizeType const& size)
    : buffer_(std::make_unique<ByteType>(size))
  { }

  virtual SerialByteType* getBuffer() const override {
    return buffer_.get();
  }

private:
  std::unique_ptr<ByteType> buffer_ = nullptr;
};

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_MANAGED_BUFFER*/
