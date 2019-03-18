
#if !defined INCLUDED_SERDES_USER_BUFFER
#define INCLUDED_SERDES_USER_BUFFER

#include "serdes_common.h"
#include "buffer.h"

namespace serdes {

struct UserBuffer : Buffer {
  UserBuffer(SerialByteType* ptr, SerialSizeType const& size)
    : size_(size), buffer_(ptr)
  { }

  virtual SerialByteType* getBuffer() const override {
    return buffer_;
  }

  virtual SerialSizeType getSize() const override {
    return size_;
  }

private:
  SerialSizeType size_ = 0;

  SerialByteType* buffer_ = nullptr;
};

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_USER_BUFFER*/
