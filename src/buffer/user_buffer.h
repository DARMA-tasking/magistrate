
#if !defined INCLUDED_SERDES_USER_BUFFER
#define INCLUDED_SERDES_USER_BUFFER

#include "serdes_common.h"

namespace serdes {

struct UserBuffer : Buffer {
  UserBuffer(SerialByteType* ptr) : buffer_(ptr) { }

  virtual SerialByteType* getBuffer() const override {
    return buffer_;
  }

private:
  SerialByteType* buffer_ = nullptr;
};

} /* end namespace serdes */

#endif /*INCLUDED_SERDES_USER_BUFFER*/
